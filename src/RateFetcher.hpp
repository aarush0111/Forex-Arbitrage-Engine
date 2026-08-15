#pragma once

#include <string>
#include <vector>
#include <utility>

// Fetches USD-based exchange rates from the Open Exchange Rates API and
// converts them into all directed currency-to-currency pairs.
class RateFetcher {
public:
    struct Rate {
        std::string from;
        std::string to;
        double rate;
    };

    // Fetch live rates for the given app id. Throws std::runtime_error on
    // network or parse failure.
    static std::vector<Rate> fetchLive(const std::string& appId);

    // Load rates from a local JSON file in Open Exchange Rates format
    // (a top-level "rates" object of CODE -> USD-per-unit).
    static std::vector<Rate> fetchFromFile(const std::string& path);

private:
    // Turn a USD-based { "EUR": 0.92, "GBP": 0.79, ... } table into every
    // directed pair by cross rate: rate(A,B) = usdRate[B] / usdRate[A].
    static std::vector<Rate> crossRates(
        const std::vector<std::pair<std::string, double>>& usdRates);
};
