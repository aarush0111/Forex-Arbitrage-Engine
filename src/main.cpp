#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "Graph.hpp"
#include "RateFetcher.hpp"

namespace {

const char* kResultPath = "data/arbitrage_result.json";

void writeResult(const Graph& g, const std::vector<std::string>& cycle) {
    std::ofstream out(kResultPath);
    if (!out) {
        std::cerr << "warning: could not write " << kResultPath << "\n";
        return;
    }
    out << "{\n";
    out << "  \"arbitrage\": " << (cycle.empty() ? "false" : "true") << ",\n";

    if (!cycle.empty()) {
        double product = g.cycleProduct(cycle);
        out << "  \"profit_factor\": " << std::setprecision(10) << product << ",\n";
        out << "  \"cycle\": [";
        for (size_t i = 0; i < cycle.size(); ++i) {
            out << "\"" << cycle[i] << "\"";
            if (i + 1 < cycle.size()) out << ", ";
        }
        out << "]\n";
    } else {
        out << "  \"cycle\": []\n";
    }
    out << "}\n";
}

} // namespace

int main(int argc, char** argv) {
    bool offline = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--offline") offline = true;
    }

    std::vector<RateFetcher::Rate> rates;
    try {
        if (offline) {
            std::cout << "Loading rates from data/sample_rates.json\n";
            rates = RateFetcher::fetchFromFile("data/sample_rates.json");
        } else {
            const char* appId = std::getenv("OXR_APP_ID");
            std::cout << "Fetching live rates from Open Exchange Rates\n";
            rates = RateFetcher::fetchLive(appId ? appId : "");
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        std::cerr << "hint: try --offline to use the bundled sample data\n";
        return 1;
    }

    Graph graph;
    for (const auto& r : rates) {
        graph.addRate(r.from, r.to, r.rate);
    }
    std::cout << "Built graph with " << graph.currencyCount()
              << " currencies and " << rates.size() << " directed edges\n";

    std::vector<std::string> cycle = graph.findArbitrageCycle();

    if (cycle.empty()) {
        std::cout << "No arbitrage opportunity detected.\n";
    } else {
        double product = graph.cycleProduct(cycle);
        std::cout << "Arbitrage cycle found (profit factor "
                  << std::setprecision(8) << product << "):\n  ";
        for (size_t i = 0; i < cycle.size(); ++i) {
            std::cout << cycle[i];
            if (i + 1 < cycle.size()) std::cout << " -> ";
        }
        std::cout << "\n";
        std::cout << "Start with 1.0 unit, end with " << product
                  << " units (" << (product - 1.0) * 100.0 << "% gain)\n";
    }

    writeResult(graph, cycle);
    std::cout << "Wrote " << kResultPath << "\n";
    return 0;
}
