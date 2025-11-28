# Quant Finance Toolkit  
Author: John Jacobson

This repo is a small collection of projects I wrote while learning more about options, volatility surfaces, microstructure, and systematic trading. Everything here is original and kept intentionally simple so I can experiment and understand the mechanics clearly.

## What's included

### Volatility Surface Arbitrage (C++)
`include/vol_surface_arbitrage.h`  
Checks for basic no-arbitrage violations across strikes and maturities using Black–Scholes pricing. Covers butterfly and calendar spread conditions.

### Options Pricing & Greeks (C++)
`include/options_greeks.h`  
Black–Scholes call/put pricing, analytical Greeks, and a Newton–Raphson implied volatility solver.

### Limit Order Book Simulator (C++)
`include/orderbook_simulator.h`  
A small price-time priority matching engine with partial fills. Useful for studying simple execution behavior and order flow.

### Backtesting Framework (Python)
`python/backtesting_framework.py`  
A compact backtester that takes a strategy signal and produces returns and an equity curve.

### Example Strategies (Python)
`python/backtesting_examples.py`  
Moving-average crossover and mean-reversion examples built on top of the backtesting framework.

### C++ Examples
`examples.cpp`  
Shows how the C++ modules work together.

## Build (C++)

```bash
mkdir build && cd build
cmake ..
make
./examples
