/**
 * @file examples.cpp
 * @author John Jacobson
 * @brief Example driver for the Quant Finance Toolkit C++ modules.
 *
 * This file runs a few small demonstrations of:
 *   - Volatility surface arbitrage checks
 *   - Black–Scholes Greeks and implied volatility
 *   - A simple limit order book simulation
 */

#include <iostream>
#include <vector>

#include "include/vol_surface_arbitrage.h"
#include "include/options_greeks.h"
#include "include/orderbook_simulator.h"

int main() {
    using namespace qf;

    // ===============================
    // 1. Volatility surface example
    // ===============================
    {
        std::cout << "=== Volatility Surface Arbitrage Example ===\n";

        std::vector<OptionQuote> quotes;

        // Simple synthetic surface around spot = 100
        quotes.push_back({100.0, 0.5, 0.20, 'C', 4.8, 5.2, 100.0, 0.01});
        quotes.push_back({100.0, 1.0, 0.25, 'C', 7.8, 8.2, 100.0, 0.01});
        quotes.push_back({ 90.0, 0.5, 0.22, 'C', 11.8, 12.2, 100.0, 0.01});
        quotes.push_back({110.0, 0.5, 0.19, 'C',  1.8,  2.2, 100.0, 0.01});

        VolSurfaceArbitrageDetector detector;
        auto arbs = detector.detect_arbitrage(quotes);

        if (arbs.empty()) {
            std::cout << "No arbitrage flags found with these simple checks.\n\n";
        } else {
            std::cout << "Found " << arbs.size() << " potential arbitrage flags:\n";
            for (const auto& a : arbs) {
                std::cout << "  Type: " << a.type
                          << " | " << a.description << "\n";
            }
            std::cout << "\n";
        }
    }

    // ===============================
    // 2. Greeks and implied vol
    // ===============================
    {
        std::cout << "=== Greeks and Implied Vol Example ===\n";

        double S = 100.0;
        double K = 100.0;
        double T = 1.0;
        double r = 0.01;
        double sigma = 0.20;

        double call_price = bs_call(S, K, T, r, sigma);
        Greeks g = call_greeks(S, K, T, r, sigma);

        std::cout << "Call price: " << call_price << "\n";
        std::cout << "Delta: " << g.delta
                  << ", Gamma: " << g.gamma
                  << ", Vega: " << g.vega
                  << ", Theta: " << g.theta
                  << ", Rho: " << g.rho << "\n";

        try {
            double implied = implied_vol_call(call_price, S, K, T, r);
            std::cout << "Implied vol recovered from price: " << implied << "\n\n";
        } catch (const std::exception& e) {
            std::cout << "Implied vol solver error: " << e.what() << "\n\n";
        }
    }

    // ===============================
    // 3. Order book simulator
    // ===============================
    {
        std::cout << "=== Order Book Simulator Example ===\n";

        OrderBook ob;

        auto [id1, trades1] = ob.add_limit_order(Side::Buy, 99.0, 100);
        auto [id2, trades2] = ob.add_limit_order(Side::Buy, 98.5, 200);
        auto [id3, trades3] = ob.add_limit_order(Side::Sell, 101.0, 150);

        std::cout << "After initial orders:\n";
        std::cout << "  Best bid: "
                  << (ob.best_bid().has_value() ? std::to_string(*ob.best_bid()) : "none") << "\n";
        std::cout << "  Best ask: "
                  << (ob.best_ask().has_value() ? std::to_string(*ob.best_ask()) : "none") << "\n";

        // Now send an aggressive buy that crosses the spread
        auto [id4, trades4] = ob.add_limit_order(Side::Buy, 102.0, 300);

        std::cout << "\nAggressive buy at 102.0 generated " << trades4.size() << " trade(s):\n";
        for (const auto& t : trades4) {
            std::cout << "  Trade: buy_id=" << t.buy_id
                      << ", sell_id=" << t.sell_id
                      << ", px=" << t.price
                      << ", qty=" << t.quantity << "\n";
        }

        std::cout << "\nFinal book state:\n";
        std::cout << "  Best bid: "
                  << (ob.best_bid().has_value() ? std::to_string(*ob.best_bid()) : "none") << "\n";
        std::cout << "  Best ask: "
                  << (ob.best_ask().has_value() ? std::to_string(*ob.best_ask()) : "none") << "\n\n";
    }

    return 0;
}
