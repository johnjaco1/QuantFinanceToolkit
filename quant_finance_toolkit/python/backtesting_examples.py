"""
backtesting_examples.py
Author: John Jacobson

Example strategies for the Backtester class.

This file demonstrates:
    - A simple moving average crossover strategy
    - A mean-reversion z-score strategy

Both return {-1, 0, 1} signals and run through the Backtester.
"""

import numpy as np
import pandas as pd

from backtesting_framework import Backtester


# ============================================
# 1. Moving Average Crossover Strategy
# ============================================

def ma_crossover_strategy(prices, short=20, long=50):
    prices = pd.Series(prices)
    ma_s = prices.rolling(short).mean()
    ma_l = prices.rolling(long).mean()

    signals = (ma_s > ma_l).astype(int) - (ma_s < ma_l).astype(int)
    return signals.fillna(0).values


# ============================================
# 2. Mean Reversion Z-Score Strategy
# ============================================

def mean_reversion_strategy(prices, lookback=20, z_entry=1.0):
    prices = pd.Series(prices)
    mean = prices.rolling(lookback).mean()
    std = prices.rolling(lookback).std()

    z = (prices - mean) / std

    # long when below -z_entry, short when above +z_entry
    signals = np.where(z < -z_entry, 1,
               np.where(z > z_entry, -1, 0))

    return signals


# ============================================
# 3. Example runner
# ============================================

if __name__ == "__main__":
    # Generate simple synthetic price data
    rng = np.random.default_rng(42)
    prices = 100 + np.cumsum(rng.normal(0, 1, 500))

    print("=== MA Crossover Strategy ===")
    bt1 = Backtester(prices, ma_crossover_strategy)
    strat_rets, equity, summary = bt1.run()
    print(summary)

    print("\n=== Mean Reversion Strategy ===")
    bt2 = Backtester(prices, mean_reversion_strategy)
    strat_rets2, equity2, summary2 = bt2.run()
    print(summary2)
