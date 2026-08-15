# Graph-Based Forex Arbitrage Modeling

A forex arbitrage detection engine that models exchange rates between 30+ currencies
as a directed graph and uses the **Bellman–Ford** algorithm (over log-transformed
rates) to uncover arbitrage cycles: sequences of trades that return more of a
currency than you started with.

- **C++ engine** (`src/`) fetches live rates from the [Open Exchange Rates API](https://openexchangerates.org/),
  builds a directed weighted graph, and detects negative-weight cycles.
- **Python visualizer** (`viz/`) draws the currency graph and highlights any
  arbitrage cycle using **NetworkX** and **Matplotlib**.

## The idea

For an arbitrage opportunity we want a cycle of currencies

```
c1 -> c2 -> c3 -> ... -> c1
```

whose product of exchange rates is greater than 1:

```
rate(c1,c2) * rate(c2,c3) * ... * rate(ck,c1) > 1
```

Taking `-log` of each rate turns the product into a sum:

```
sum( -log(rate) ) < 0
```

So an arbitrage opportunity is exactly a **negative-weight cycle** in the graph
where each edge weight is `-log(rate)`. Bellman–Ford finds negative cycles, so it
finds arbitrage.

## Build (C++ engine)

Requires a C++17 compiler and [libcurl](https://curl.se/libcurl/) +
[nlohmann/json](https://github.com/nlohmann/json).

```bash
cd "Forex Arbitrage Model"
cmake -S . -B build
cmake --build build
```

Run with your Open Exchange Rates app id (get a free one at openexchangerates.org):

```bash
export OXR_APP_ID=your_app_id_here
./build/forex_arbitrage
```

Add `--offline` to use the bundled `data/sample_rates.json` instead of the API:

```bash
./build/forex_arbitrage --offline
```

The engine writes any detected cycle to `data/arbitrage_result.json`.

## Visualize (Python)

```bash
cd viz
pip install -r requirements.txt
python visualize.py
```

This reads `data/arbitrage_result.json` and renders the currency graph with the
arbitrage path highlighted.

## Layout

```
Forex Arbitrage Model/
├── CMakeLists.txt
├── src/
│   ├── main.cpp          # entry point / CLI
│   ├── Graph.hpp/.cpp    # directed weighted graph + Bellman–Ford
│   ├── RateFetcher.hpp/.cpp  # Open Exchange Rates client (libcurl)
│   └── json.hpp          # (vendor nlohmann/json here, see note)
├── data/
│   └── sample_rates.json # offline sample for --offline mode
└── viz/
    ├── visualize.py
    └── requirements.txt
```

> **Note:** `src/json.hpp` is expected to be the single-header
> [nlohmann/json](https://github.com/nlohmann/json/releases). Download
> `json.hpp` and drop it in `src/`, or install it via your package manager.
