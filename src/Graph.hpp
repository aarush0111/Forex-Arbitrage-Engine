#pragma once

#include <string>
#include <vector>
#include <unordered_map>

// Directed weighted graph of currencies.
//
// Each currency is a node. An edge from -> to carries weight -log(rate),
// so a negative-weight cycle is exactly an arbitrage opportunity.
// (See the write-up, sections 4 and 6, for why.)
class Graph {
public:
    // Register a currency, returning its index (does nothing if it exists).
    int addCurrency(const std::string& code);

    // Add a directed edge with the given exchange rate (rate must be > 0).
    void addRate(const std::string& from, const std::string& to, double rate);

    int currencyCount() const { return static_cast<int>(codes_.size()); }
    const std::string& codeAt(int index) const { return codes_[index]; }

    // Run Bellman-Ford and return the currencies forming an arbitrage cycle
    // (c1, c2, ..., c1), or an empty vector if none exists.
    std::vector<std::string> findArbitrageCycle() const;

    // Product of the exchange rates around a cycle (> 1 for real arbitrage).
    double cycleProduct(const std::vector<std::string>& cycle) const;

private:
    struct Edge {
        int from;
        int to;
        double weight; // -log(rate)
    };

    // Walk predecessor pointers back into the cycle and collect it.
    std::vector<std::string> reconstructCycle(int start,
                                              const std::vector<int>& pred) const;

    std::vector<std::string> codes_;
    std::unordered_map<std::string, int> index_;
    std::vector<Edge> edges_;
    // rate_[from][to] = exchange rate, for cycleProduct lookups.
    std::unordered_map<std::string, std::unordered_map<std::string, double>> rate_;
};
