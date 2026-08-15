"""Draw the currency graph and highlight the arbitrage loop the engine found.

Reads data/arbitrage_result.json plus the rates it ran on, colours the cycle
red, and saves a PNG.
"""

import json
import os
import sys

import matplotlib.pyplot as plt
import networkx as nx

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
RESULT_PATH = os.path.join(ROOT, "data", "arbitrage_result.json")
RATES_PATH = os.path.join(ROOT, "data", "sample_rates.json")


def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def build_graph(rates_doc):
    g = nx.DiGraph()
    if "pairs" in rates_doc:
        for p in rates_doc["pairs"]:
            g.add_edge(p["from"], p["to"], rate=p["rate"])
    elif "rates" in rates_doc:
        codes = list(rates_doc["rates"].items())
        for a, ra in codes:
            for b, rb in codes:
                if a != b and ra > 0:
                    g.add_edge(a, b, rate=rb / ra)
    return g


def cycle_edges(cycle):
    return list(zip(cycle[:-1], cycle[1:]))


def main():
    if not os.path.exists(RESULT_PATH):
        sys.exit(
            "No result file at %s.\nRun the C++ engine first "
            "(./build/forex_arbitrage --offline)." % RESULT_PATH
        )

    result = load_json(RESULT_PATH)
    rates_doc = load_json(RATES_PATH)
    g = build_graph(rates_doc)

    cycle = result.get("cycle", [])
    highlight = set(cycle_edges(cycle)) if len(cycle) > 1 else set()
    cycle_nodes = set(cycle)

    pos = nx.circular_layout(g)

    node_colors = [
        "#ff6b6b" if n in cycle_nodes else "#4dabf7" for n in g.nodes()
    ]
    edge_colors = []
    edge_widths = []
    for u, v in g.edges():
        if (u, v) in highlight:
            edge_colors.append("#e03131")
            edge_widths.append(3.0)
        else:
            edge_colors.append("#cccccc")
            edge_widths.append(0.6)

    plt.figure(figsize=(11, 9))
    nx.draw_networkx_nodes(g, pos, node_color=node_colors, node_size=1600)
    nx.draw_networkx_labels(g, pos, font_size=11, font_weight="bold")
    nx.draw_networkx_edges(
        g,
        pos,
        edge_color=edge_colors,
        width=edge_widths,
        arrowstyle="-|>",
        arrowsize=14,
        connectionstyle="arc3,rad=0.08",
    )

    if result.get("arbitrage"):
        factor = result.get("profit_factor", 1.0)
        title = "Arbitrage cycle: %s  (profit factor %.5f, %+.3f%%)" % (
            " -> ".join(cycle),
            factor,
            (factor - 1.0) * 100.0,
        )
    else:
        title = "No arbitrage cycle detected"
    plt.title(title, fontsize=13)
    plt.axis("off")
    plt.tight_layout()

    out_path = os.path.join(ROOT, "data", "arbitrage_graph.png")
    plt.savefig(out_path, dpi=150)
    print("Saved %s" % out_path)
    plt.show()


if __name__ == "__main__":
    main()
