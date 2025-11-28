# Technical Documentation  
Author: John Jacobson

This document provides a technical overview of the components inside the Quant Finance Toolkit. The code is intentionally simple but follows the underlying mathematics and mechanics used in real quantitative finance.

---

# 1. Volatility Surface Arbitrage Detector (C++)

**File:** `include/vol_surface_arbitrage.h`

This module checks for two of the most common static no-arbitrage violations:

### 1.1 Butterfly Arbitrage (Strike Convexity)

For a fixed maturity \( T \), call prices must be convex in strike:

\[
C(K_1) - 2C(K_2) + C(K_3) \ge 0
\]

The detector computes Black–Scholes call prices for each quote and checks this finite-difference convexity.

### 1.2 Calendar Spread Arbitrage (Time Monotonicity)

For the same strike \( K \):

\[
C(T_1) \le C(T_2) \quad \text{if } T_1 < T_2
\]

This ensures longer maturities hold more time value.

### 1.3 Implementation Notes

- Internal pricing uses Black–Scholes for consistency.
- Sorting ensures proper ordering of strikes.
- Numerical tolerances are handled with a small epsilon.

This is useful for identifying inconsistent volatility inputs or malformed surfaces.

---

# 2. Options Greeks & Implied Volatility (C++)

**File:** `include/options_greeks.h`

This module implements:

### 2.1 Black–Scholes Prices

- Call and Put pricing formulas
- Normal CDF and PDF wrapped in simple inline functions

### 2.2 Greeks

All analytical Greeks are implemented:

- **Delta**
- **Gamma**
- **Vega**
- **Theta**
- **Rho**

These help illustrate sensitivity to underlying inputs.

### 2.3 Implied Volatility (IV) Solver

A Newton–Raphson root finder solves:

\[
BS(\sigma) - \text{market price} = 0
\]

Includes:

- Safeguards for zero Vega
- Bounds on sigma
- Convergence tolerance and iteration limits

---

# 3. Limit Order Book Simulator (C++)

**File:** `include/orderbook_simulator.h`

This simulator models a basic price-time priority limit order book:

### 3.1 Core Behavior

- Bids sorted high-to-low
- Asks sorted low-to-high
- Orders at the same price stored FIFO
- Partial fills supported
- Trades recorded with:
  - buy ID
  - sell ID
  - price
  - quantity

### 3.2 Matching Logic

Matches against opposite side until:

- No more crossing prices
- Incoming order is fully filled

### 3.3 Uses

This project helped me understand order queuing, best bid/ask, and crossing orders — foundational concepts in market microstructure.

---

# 4. Backtesting Framework (Python)

**File:** `python/backtesting_framework.py`

A minimal research backtesting engine with:

- Price series input
- Strategy function returning signals {−1, 0, 1}
- One-bar execution lag
- Equity curve output
- Sharpe ratio
- Max drawdown

### 4.1 Execution Lag

Trades use:

\[
\text{position}_t = \text{signal}_{t-1}
\]

This avoids unrealistic forward-looking bias.

### 4.2 Performance Metrics

- Sharpe standardized to 252 days
- Max drawdown from cumulative returns

---

# 5. Example Strategies (Python)

**File:** `python/backtesting_examples.py`

Contains two commonly taught signal types:

- Moving average crossover (trend-following)
- Mean-reversion (z-score)

These demonstrate how to use the framework.

---

# 6. Putting It Together

**File:** `examples.cpp`

Runs:

- The volatility arbitrage checks
- Greeks and implied vol
- Order book trades

This provides a simple end-to-end demonstration of the C++ modules.

---

# 7. Design Principles

Across all modules I aimed for:

- Clear mathematical grounding
- Straightforward implementation without unnecessary abstraction
- Ability to expand into more complex systems
- Realistic but readable examples

This toolkit serves as a foundation for future quant research work, including:

- Volatility surface construction
- Stochastic volatility models
- Multi-asset backtesting
- Execution algorithms
- More advanced order book modeling

---

# End of Technical Documentation
