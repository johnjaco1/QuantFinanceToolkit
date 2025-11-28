#ifndef OPTIONS_GREEKS_H
#define OPTIONS_GREEKS_H

/**
 * @file options_greeks.h
 * @author John Jacobson
 * @brief Black–Scholes pricing, analytical Greeks, and implied volatility solver.
 *
 * I wrote this module while studying the sensitivities of European options and
 * how implied volatility is backed out from observed market prices. Everything
 * is implemented directly from the Black–Scholes model for clarity.
 *
 * Included:
 *   - Black–Scholes call and put pricing
 *   - Analytical Greeks (Delta, Gamma, Vega, Theta, Rho)
 *   - Newton–Raphson implied volatility solver
 *
 * This module is used by the examples and can be extended into surfaces,
 * calibration tools, or portfolio risk analysis.
 */

#include <cmath>
#include <stdexcept>

namespace qf {

static constexpr double INV_SQRT_2PI = 0.39894228040143267794;

inline double norm_pdf(double x) {
    return INV_SQRT_2PI * std::exp(-0.5 * x * x);
}

inline double norm_cdf(double x) {
    return 0.5 * std::erfc(-x * M_SQRT1_2);
}

struct Greeks {
    double delta;
    double gamma;
    double vega;
    double theta;
    double rho;
};

// =======================
// Black–Scholes Pricing
// =======================

inline double bs_call(double S, double K, double T, double r, double sigma) {
    if (T <= 0.0)
        return std::max(S - K, 0.0);

    double st = std::sqrt(T);
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) /
                (sigma * st);
    double d2 = d1 - sigma * st;

    return S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
}

inline double bs_put(double S, double K, double T, double r, double sigma) {
    if (T <= 0.0)
        return std::max(K - S, 0.0);

    double st = std::sqrt(T);
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) /
                (sigma * st);
    double d2 = d1 - sigma * st;

    return K * std::exp(-r * T) * norm_cdf(-d2) - S * norm_cdf(-d1);
}

// =======================
// Greeks (Call)
// =======================

inline Greeks call_greeks(double S, double K, double T, double r, double sigma) {
    Greeks g{};

    if (T <= 0.0) {
        g.delta = (S > K) ? 1.0 : 0.0;
        g.gamma = 0.0;
        g.vega = 0.0;
        g.theta = 0.0;
        g.rho = 0.0;
        return g;
    }

    double st = std::sqrt(T);
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) /
                (sigma * st);
    double d2 = d1 - sigma * st;

    g.delta = norm_cdf(d1);
    g.gamma = norm_pdf(d1) / (S * sigma * st);
    g.vega = S * norm_pdf(d1) * st;
    g.theta = -(S * norm_pdf(d1) * sigma) / (2.0 * st)
              - r * K * std::exp(-r * T) * norm_cdf(d2);
    g.rho = K * T * std::exp(-r * T) * norm_cdf(d2);

    return g;
}

// =======================
// Greeks (Put)
// =======================

inline Greeks put_greeks(double S, double K, double T, double r, double sigma) {
    Greeks g{};

    if (T <= 0.0) {
        g.delta = (S < K) ? -1.0 : 0.0;
        g.gamma = 0.0;
        g.vega = 0.0;
        g.theta = 0.0;
        g.rho = 0.0;
        return g;
    }

    double st = std::sqrt(T);
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) /
                (sigma * st);
    double d2 = d1 - sigma * st;

    g.delta = norm_cdf(d1) - 1.0;
    g.gamma = norm_pdf(d1) / (S * sigma * st);
    g.vega = S * norm_pdf(d1) * st;
    g.theta = -(S * norm_pdf(d1) * sigma) / (2.0 * st)
              + r * K * std::exp(-r * T) * norm_cdf(-d2);
    g.rho = -K * T * std::exp(-r * T) * norm_cdf(-d2);

    return g;
}

// =======================
// Implied Volatility (Call)
// =======================

inline double implied_vol_call(double market_price,
                               double S, double K, double T, double r,
                               double initial_guess = 0.20,
                               double tol = 1e-6,
                               int max_iter = 100) {
    double sigma = initial_guess;

    for (int i = 0; i < max_iter; ++i) {
        double price = bs_call(S, K, T, r, sigma);
        double diff = price - market_price;

        if (std::fabs(diff) < tol)
            return sigma;

        Greeks g = call_greeks(S, K, T, r, sigma);

        if (g.vega < 1e-8)
            break;  // avoid division by zero

        sigma -= diff / g.vega;

        if (sigma <= 0.0)
            sigma = 1e-4;
    }

    throw std::runtime_error("implied_vol_call: did not converge");
}

} // namespace qf

#endif // OPTIONS_GREEKS_H
