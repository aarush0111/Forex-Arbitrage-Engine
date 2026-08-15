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

int Graph::indexOf(const std::string& code) const {
    auto it = index_.find(code);
    return it == index_.end() ? -1 : it->second;
}

void Graph::addRate(const std::string& from, const std::string& to, double rate) {
    if (rate <= 0.0) return; // log undefined; skip bad quotes
    int u = addCurrency(from);
    int v = addCurrency(to);
    edges_.push_back(Edge{u, v, -std::log(rate), rate});
    rate_[u][v] = rate;
}

double Graph::cycleProduct(const std::vector<std::string>& cycle) const {
    if (cycle.size() < 2) return 1.0;
    double product = 1.0;
    for (size_t i = 0; i + 1 < cycle.size(); ++i) {
        int u = indexOf(cycle[i]);
        int v = indexOf(cycle[i + 1]);
        if (u < 0 || v < 0) return 0.0;
        auto ui = rate_.find(u);
        if (ui == rate_.end()) return 0.0;
        auto vi = ui->second.find(v);
        if (vi == ui->second.end()) return 0.0;
        product *= vi->second;
    }
    return product;
}

std::vector<std::string> Graph::findArbitrageCycle() const {
    const int n = currencyCount();
    if (n == 0) return {};

    const double INF = std::numeric_limits<double>::infinity();
    // Single virtual source: start all distances at 0 so every component is
    // reachable (equivalent to adding a 0-weight edge to every node).
    std::vector<double> dist(n, 0.0);
    std::vector<int> pred(n, -1);

    int updatedNode = -1;

    // Only count an improvement larger than this as real. Reciprocal quotes
    // are never exact in floating point, so a "flat" round-trip lands within
    // ~1e-9 of zero weight; a genuine arbitrage is orders of magnitude bigger.
    // This threshold also stands in for ignoring sub-spread opportunities.
    const double kEps = 1e-7;

    // Relax all edges n times. The n-th pass detecting a relaxation means a
    // negative cycle is reachable.
    for (int iter = 0; iter < n; ++iter) {
        updatedNode = -1;
        for (const Edge& e : edges_) {
            if (dist[e.from] == INF) continue;
            if (dist[e.from] + e.weight < dist[e.to] - kEps) {
                dist[e.to] = dist[e.from] + e.weight;
                pred[e.to] = e.from;
                updatedNode = e.to;
            }
        }
        if (updatedNode == -1) break; // converged, no negative cycle
    }

    if (updatedNode == -1) return {};

    // The updated node may be only reachable from the cycle, not on it.
    // Walk n predecessors back to land firmly inside the cycle.
    int cur = updatedNode;
    for (int i = 0; i < n; ++i) cur = pred[cur];

    // Collect the cycle by following predecessors until we return to `cur`.
    std::vector<int> cycleIdx;
    int node = cur;
    do {
        cycleIdx.push_back(node);
        node = pred[node];
    } while (node != cur && node != -1);
    cycleIdx.push_back(cur);

    std::reverse(cycleIdx.begin(), cycleIdx.end());

    std::vector<std::string> cycle;
    cycle.reserve(cycleIdx.size());
    for (int idx : cycleIdx) cycle.push_back(codes_[idx]);
    return cycle;
}
