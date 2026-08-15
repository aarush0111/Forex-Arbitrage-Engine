#include "Graph.hpp"

#include <cmath>
#include <limits>
#include <algorithm>

int Graph::addCurrency(const std::string& code) {
    auto it = index_.find(code);
    if (it != index_.end()) return it->second;
    int idx = static_cast<int>(codes_.size());
    codes_.push_back(code);
    index_[code] = idx;
    return idx;
}

void Graph::addRate(const std::string& from, const std::string& to, double rate) {
    if (rate <= 0.0) return; // log is undefined for non-positive rates
    addCurrency(from);
    addCurrency(to);
    edges_.push_back(Edge{index_[from], index_[to], -std::log(rate)});
    rate_[from][to] = rate;
}

// Bellman-Ford with negative-cycle detection (edge-list version).
// Follows Listing 1 of the write-up.
std::vector<std::string> Graph::findArbitrageCycle() const {
    int V = currencyCount();
    if (V == 0) return {};

    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> dist(V, INF); // best known distance to each node
    std::vector<int> pred(V, -1);     // predecessor, for cycle reconstruction
    dist[0] = 0.0;                    // use the first currency as the source

    // A tiny tolerance: exchange rates are only ever consistent to within
    // floating-point rounding, so require an improvement bigger than this to
    // avoid mistaking rounding noise for a (fake) negative cycle.
    const double EPS = 1e-9;

    // Phase 1: relax all edges V - 1 times.
    for (int pass = 0; pass < V - 1; ++pass) {
        for (const Edge& e : edges_) {
            if (dist[e.from] != INF &&
                dist[e.from] + e.weight < dist[e.to] - EPS) {
                dist[e.to] = dist[e.from] + e.weight;
                pred[e.to] = e.from;
            }
        }
    }

    // Phase 2: one more pass. If anything still improves, a negative cycle
    // (an arbitrage opportunity) is present.
    for (const Edge& e : edges_) {
        if (dist[e.from] != INF &&
            dist[e.from] + e.weight < dist[e.to] - EPS) {
            return reconstructCycle(e.to, pred);
        }
    }
    return {}; // no arbitrage
}

// Recover the currency loop from predecessor pointers. Follows Listing 2.
std::vector<std::string> Graph::reconstructCycle(
        int start, const std::vector<int>& pred) const {
    int V = currencyCount();

    // The improving node may only lead into the cycle, so step back V times
    // to make sure we are standing on it.
    int x = start;
    for (int i = 0; i < V; ++i) x = pred[x];

    // Walk predecessors until we return to x; that loop is the cycle.
    std::vector<int> cycle;
    int cur = x;
    do {
        cycle.push_back(cur);
        cur = pred[cur];
    } while (cur != x);
    cycle.push_back(x);

    std::reverse(cycle.begin(), cycle.end()); // present in trading order

    std::vector<std::string> codes;
    codes.reserve(cycle.size());
    for (int idx : cycle) codes.push_back(codes_[idx]);
    return codes;
}

double Graph::cycleProduct(const std::vector<std::string>& cycle) const {
    if (cycle.size() < 2) return 1.0;
    double product = 1.0;
    for (size_t i = 0; i + 1 < cycle.size(); ++i) {
        auto from = rate_.find(cycle[i]);
        if (from == rate_.end()) return 0.0;
        auto to = from->second.find(cycle[i + 1]);
        if (to == from->second.end()) return 0.0;
        product *= to->second;
    }
    return product;
}
