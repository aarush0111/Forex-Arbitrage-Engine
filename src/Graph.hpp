#pragma once

#include <string>
#include <vector>
#include <unordered_map>

// Currency graph. Each edge weight is -log(rate), which makes an arbitrage
// loop show up as a negative-weight cycle.
class Graph {
public:
    int addCurrency(const std::string& code);
    void addRate(const std::string& from, const std::string& to, double rate);

    int currencyCount() const { return static_cast<int>(codes_.size()); }
    const std::string& codeAt(int index) const { return codes_[index]; }

    // The arbitrage loop, e.g. {USD, EUR, GBP, USD}. Empty if there isn't one.
    std::vector<std::string> findArbitrageCycle() const;

    // Multiply the rates around a cycle; > 1 means real profit.
    double cycleProduct(const std::vector<std::string>& cycle) const;

private:
    struct Edge {
        int from;
        int to;
        double weight; // -log(rate)
    };

    std::vector<std::string> reconstructCycle(int start,
                                              const std::vector<int>& pred) const;

    std::vector<std::string> codes_;
    std::unordered_map<std::string, int> index_;
    std::vector<Edge> edges_;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> rate_;
};
