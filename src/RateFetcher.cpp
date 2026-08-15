#include "RateFetcher.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <curl/curl.h>
#include "json.hpp"

using json = nlohmann::json;

namespace {

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), total);
    return total;
}

std::string httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("failed to init libcurl");

    std::string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("request failed: ") +
                                 curl_easy_strerror(res));
    }
    if (httpCode != 200) {
        throw std::runtime_error("HTTP " + std::to_string(httpCode) +
                                 " from Open Exchange Rates");
    }
    return body;
}

std::vector<std::pair<std::string, double>> parseUsdRates(const std::string& text) {
    json doc = json::parse(text);
    if (!doc.contains("rates")) {
        throw std::runtime_error("response has no \"rates\" field");
    }
    std::vector<std::pair<std::string, double>> out;
    for (auto it = doc["rates"].begin(); it != doc["rates"].end(); ++it) {
        out.emplace_back(it.key(), it.value().get<double>());
    }
    return out;
}

} // namespace

std::vector<RateFetcher::Rate> RateFetcher::crossRates(
    const std::vector<std::pair<std::string, double>>& usdRates) {
    std::vector<Rate> rates;
    for (const auto& a : usdRates) {
        for (const auto& b : usdRates) {
            if (a.first == b.first) continue;
            if (a.second <= 0.0) continue;
            // usdRates[X] = units of X per 1 USD.
            // rate(A -> B) = (USD per A) inverse applied: B_per_USD / A_per_USD.
            rates.push_back(Rate{a.first, b.first, b.second / a.second});
        }
    }
    return rates;
}

std::vector<RateFetcher::Rate> RateFetcher::fetchLive(const std::string& appId) {
    if (appId.empty()) {
        throw std::runtime_error("missing Open Exchange Rates app id "
                                 "(set OXR_APP_ID)");
    }
    const std::string url =
        "https://openexchangerates.org/api/latest.json?app_id=" + appId;
    return crossRates(parseUsdRates(httpGet(url)));
}

std::vector<RateFetcher::Rate> RateFetcher::fetchFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open " + path);
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string text = ss.str();

    json doc = json::parse(text);

    // Format 1: explicit directed pairs. Lets the sample data carry a real
    // (planted) inconsistency so the detector has something to find.
    if (doc.contains("pairs")) {
        std::vector<Rate> rates;
        for (const auto& p : doc["pairs"]) {
            rates.push_back(Rate{p.at("from").get<std::string>(),
                                 p.at("to").get<std::string>(),
                                 p.at("rate").get<double>()});
        }
        return rates;
    }

    // Format 2: Open Exchange Rates USD-base table -> derived cross rates.
    return crossRates(parseUsdRates(text));
}
