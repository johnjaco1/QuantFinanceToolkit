"""
backtesting_framework.py
Author: John Jacobson

A small event-driven backtesting framework used to test simple trading ideas.
This is intentionally lightweight so I can experiment with trading logic,
returns, execution assumptions, and basic performance metrics.

The engine assumes:
    - Input price series is a pandas Series or list-like
    - Strategy outputs signals in {-1, 0, 1}
    - Trades execute on next bar (one-bar execution lag)

Outputs:
    - Returns series
    - Equity curve
    - Summary performance stats (Sharpe, drawdown)
"""

import numpy as np
import pandas as pd


class Backtester:
    def __init__(self, prices, strategy_func):
        """
        prices: pandas Series or list-like
        strategy_func: function(prices) -> signal series in {-1, 0, 1}
        """
        self.prices = pd.Series(prices).astype(float)
        self.strategy_func = strategy_func

    def compute_returns(self):
        return self.prices.pct_change().fillna(0.0)

    def run(self):
        rets = self.compute_returns()
        signals = pd.Series(self.strategy_func(self.prices), index=self.prices.index)

        # one-bar execution lag
        shifted_signals = signals.shift(1).fillna(0.0)

        strat_rets = shifted_signals * rets
        equity = (1 + strat_rets).cumprod()

        summary = {
            "total_return": float(equity.iloc[-1] - 1),
            "sharpe": self._sharpe(strat_rets),
            "max_drawdown": self._max_drawdown(equity),
        }

        return strat_rets, equity, summary

    def _sharpe(self, rets, risk_free=0.0):
        excess = rets - risk_free / 252
        if excess.std() == 0:
            return 0.0
        return float(np.sqrt(252) * excess.mean() / excess.std())

    def _max_drawdown(self, equity):
        roll_max = equity.cummax()
        dd = equity / roll_max - 1.0
        return float(dd.min())
