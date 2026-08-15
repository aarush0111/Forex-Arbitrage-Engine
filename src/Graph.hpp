#pragma once

#include <string>
#include <vector>
#include <unordered_map>

// Directed weighted graph of currencies.
//
// Each currency is a node. An edge u -> v carries weight -log(rate(u, v)),
// where rate(u, v) is how many units of v you get for one unit of u.
// A negative-weight cycle therefore corresponds to an arbitrage opportunity.
class Graph {
public:
    // Register a currency, returning its integer index (idempotent).
    int addCurrency(const std::string& code);

    // Add a directed edge with the given exchange rate. Rate must be > 0.
    void addRate(const std::string& from, const std::string& to, double rate);

    int currencyCount() const { return static_cast<int>(codes_.size()); }
    const std::string& codeAt(int index) const { return codes_[index]; }

    // Run Bellman-Ford and return the currency codes forming an arbitrage
    // cycle (c1, c2, ..., ck, c1), or an empty vector if none exists.
    std::vector<std::string> findArbitrageCycle() const;

    // Product of the exchange rates around a cycle of currency codes.
    // For a real arbitrage cycle this is > 1.
    double cycleProduct(const std::vector<std::string>& cycle) const;

private:
    struct Edge {
        int from;
        int to;
        double weight; // -log(rate)
        double rate;
    };

    int indexOf(const std::string& code) const;

    std::vector<std::string> codes_;
    std::unordered_map<std::string, int> index_;
    std::vector<Edge> edges_;
    // rate_[u][v] = exchange rate u -> v, for cycleProduct lookups.
    std::unordered_map<int, std::unordered_map<int, double>> rate_;
};
