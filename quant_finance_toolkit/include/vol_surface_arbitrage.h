#ifndef VOL_SURFACE_ARBITRAGE_H
#define VOL_SURFACE_ARBITRAGE_H

/**
 * @file vol_surface_arbitrage.h
 * @author John Jacobson
 * @brief Volatility Surface Arbitrage Detection Engine (C++17)
 *
 * This is an original project I built while studying implied volatility surfaces
 * and no-arbitrage conditions. It detects static arbitrage across strikes and
 * maturities based on the classical conditions found in Gatheral (2006) and
 * Fengler (2009).
 *
 * The detector checks:
 *   - Butterfly arbitrage (convexity across strikes)
 *   - Calendar spread arbitrage (time-value monotonicity)
 *
 * The implementation uses Black-Scholes call prices for internal consistency.
 */

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <map>
#include <stdexcept>

struct OptionQuote {
    double strike;
    double maturity;   // Time to expiration in years
    double implied_vol;
    char option_type;  // 'C' or 'P'
    double bid;
    double ask;
    double spot;
    double rate;
};

struct ArbitrageOpportunity {
    std::string type;
    std::string description;
    double severity;  // Currently unused placeholder
    std::vector<OptionQuote> involved;
};

class VolSurfaceArbitrageDetector {
private:
    double eps = 1e-6;

    double norm_cdf(double x) const {
        return 0.5 * std::erfc(-x * M_SQRT1_2);
    }

    double bs_call(double S, double K, double T, double r, double sigma) const {
        if (T <= 0.0)
            return std::max(S - K, 0.0);

        double st = std::sqrt(T);
        double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) /
                    (sigma * st);
        double d2 = d1 - sigma * st;

        return S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
    }

    bool check_butterfly(const std::vector<OptionQuote>& opts) const {
        if (opts.size() < 3) return true;

        auto sorted = opts;
        std::sort(sorted.begin(), sorted.end(),
                  [](const OptionQuote& a, const OptionQuote& b) {
                      return a.strike < b.strike;
                  });

        for (size_t i = 0; i + 2 < sorted.size(); ++i) {
            const auto& o1 = sorted[i];
            const auto& o2 = sorted[i + 1];
            const auto& o3 = sorted[i + 2];

            double C1 = bs_call(o1.spot, o1.strike, o1.maturity, o1.rate, o1.implied_vol);
            double C2 = bs_call(o2.spot, o2.strike, o2.maturity, o2.rate, o2.implied_vol);
            double C3 = bs_call(o3.spot, o3.strike, o3.maturity, o3.rate, o3.implied_vol);

            double curvature = C1 - 2.0 * C2 + C3;

            if (curvature < -eps)
                return false;
        }

        return true;
    }

    bool check_calendar(const OptionQuote& t1,
                        const OptionQuote& t2) const {
        if (std::abs(t1.strike - t2.strike) > eps)
            return true;

        double C1 = bs_call(t1.spot, t1.strike, t1.maturity, t1.rate, t1.implied_vol);
        double C2 = bs_call(t2.spot, t2.strike, t2.maturity, t2.rate, t2.implied_vol);

        return C1 <= C2 + eps;
    }

public:
    std::vector<ArbitrageOpportunity>
    detect_arbitrage(const std::vector<OptionQuote>& quotes) const {

        std::vector<ArbitrageOpportunity> found;

        std::map<double, std::vector<OptionQuote>> by_maturity;
        for (const auto& q : quotes)
            by_maturity[q.maturity].push_back(q);

        for (const auto& kv : by_maturity) {
            if (!check_butterfly(kv.second)) {
                ArbitrageOpportunity a;
                a.type = "BUTTERFLY";
                a.description = "Strike convexity violation at T=" + std::to_string(kv.first);
                a.involved = kv.second;
                found.push_back(a);
            }
        }

        for (size_t i = 0; i < quotes.size(); ++i) {
            for (size_t j = i + 1; j < quotes.size(); ++j) {
                const auto& a = quotes[i];
                const auto& b = quotes[j];
                if (a.maturity < b.maturity) {
                    if (!check_calendar(a, b)) {
                        ArbitrageOpportunity arb;
                        arb.type = "CALENDAR";
                        arb.description = "Calendar spread violation detected";
                        arb.involved = {a, b};
                        found.push_back(arb);
                    }
                }
            }
        }

        return found;
    }

    bool is_arbitrage_free(const std::vector<OptionQuote>& quotes) const {
        return detect_arbitrage(quotes).empty();
    }
};

#endif // VOL_SURFACE_ARBITRAGE_H
