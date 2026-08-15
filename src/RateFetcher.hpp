#pragma once

#include <string>
#include <vector>
#include <utility>

// Pulls USD-based rates from Open Exchange Rates and expands them into every
// currency-to-currency pair.
class RateFetcher {
public:
    struct Rate {
        std::string from;
        std::string to;
        double rate;
    };

    static std::vector<Rate> fetchLive(const std::string& appId);
    static std::vector<Rate> fetchFromFile(const std::string& path);

private:
    // rate(A, B) = usd[B] / usd[A]
    static std::vector<Rate> crossRates(
        const std::vector<std::pair<std::string, double>>& usdRates);
};
