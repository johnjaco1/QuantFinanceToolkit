#ifndef ORDERBOOK_SIMULATOR_H
#define ORDERBOOK_SIMULATOR_H

/**
 * @file orderbook_simulator.h
 * @author John Jacobson
 * @brief Simple C++17 limit order book with price-time priority matching.
 *
 * I wrote this order book simulator to understand basic market microstructure
 * concepts such as price-time priority, partial fills, order queuing, and
 * best bid/ask dynamics. The goal here is clarity and correctness rather than
 * exchange-level complexity.
 *
 * Supports:
 *   - Limit orders (buy and sell)
 *   - FIFO matching at each price level
 *   - Partial fills
 *   - Best bid/ask querying
 *   - Order cancellation by ID
 */

#include <map>
#include <deque>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace qf {

enum class Side {
    Buy,
    Sell
};

struct Trade {
    std::uint64_t buy_id;
    std::uint64_t sell_id;
    double price;
    std::uint64_t quantity;
};

struct Order {
    std::uint64_t id;
    Side side;
    double price;
    std::uint64_t quantity;
    std::uint64_t remaining;
    std::uint64_t sequence;  // used for FIFO time priority
};

class OrderBook {
private:
    struct Descending {
        bool operator()(double a, double b) const {
            return a > b;
        }
    };

    // Bids: highest price first
    std::map<double, std::deque<Order>, Descending> bids_;

    // Asks: lowest price first
    std::map<double, std::deque<Order>> asks_;

    // Order lookup (ID → pointer)
    std::unordered_map<std::uint64_t, Order*> index_;

    std::uint64_t next_id_ = 1;
    std::uint64_t next_seq_ = 1;

    void add_to_book(Order&& o) {
        auto& book = (o.side == Side::Buy ? bids_ : asks_);
        auto& queue = book[o.price];
        queue.push_back(o);
        index_[o.id] = &queue.back();
    }

    void clean_price_level(std::map<double, std::deque<Order>>& book, double price) {
        auto it = book.find(price);
        if (it != book.end() && it->second.empty())
            book.erase(it);
    }

    void match_buy(Order& incoming, std::vector<Trade>& trades) {
        while (incoming.remaining > 0 && !asks_.empty()) {
            auto it = asks_.begin();
            double ask_price = it->first;

            if (incoming.price < ask_price)
                break;

            auto& queue = it->second;
            while (incoming.remaining > 0 && !queue.empty()) {
                Order& resting = queue.front();
                std::uint64_t qty = std::min(incoming.remaining, resting.remaining);

                trades.push_back({
                    incoming.id, resting.id, ask_price, qty
                });

                incoming.remaining -= qty;
                resting.remaining -= qty;

                if (resting.remaining == 0) {
                    index_.erase(resting.id);
                    queue.pop_front();
                }
            }

            if (queue.empty())
                asks_.erase(it);
        }
    }

    void match_sell(Order& incoming, std::vector<Trade>& trades) {
        while (incoming.remaining > 0 && !bids_.empty()) {
            auto it = bids_.begin();
            double bid_price = it->first;

            if (incoming.price > bid_price)
                break;

            auto& queue = it->second;
            while (incoming.remaining > 0 && !queue.empty()) {
                Order& resting = queue.front();
                std::uint64_t qty = std::min(incoming.remaining, resting.remaining);

                trades.push_back({
                    resting.id, incoming.id, bid_price, qty
                });

                incoming.remaining -= qty;
                resting.remaining -= qty;

                if (resting.remaining == 0) {
                    index_.erase(resting.id);
                    queue.pop_front();
                }
            }

            if (queue.empty())
                bids_.erase(it);
        }
    }

public:
    std::uint64_t next_order_id() const {
        return next_id_;
    }

    /**
     * @brief Submit a new limit order.
     */
    std::pair<std::uint64_t, std::vector<Trade>>
    add_limit_order(Side side, double price, std::uint64_t quantity) {
        Order incoming;
        incoming.id = next_id_++;
        incoming.side = side;
        incoming.price = price;
        incoming.quantity = quantity;
        incoming.remaining = quantity;
        incoming.sequence = next_seq_++;

        std::vector<Trade> trades;

        if (side == Side::Buy)
            match_buy(incoming, trades);
        else
            match_sell(incoming, trades);

        if (incoming.remaining > 0)
            add_to_book(std::move(incoming));

        return {incoming.id, trades};
    }

    /**
     * @brief Cancel an existing order by ID.
     */
    bool cancel_order(std::uint64_t id) {
        auto it = index_.find(id);
        if (it == index_.end())
            return false;

        Order* ptr = it->second;

        auto& book = (ptr->side == Side::Buy ? bids_ : asks_);
        auto level = book.find(ptr->price);
        if (level == book.end())
            return false;

        auto& queue = level->second;
        for (auto qi = queue.begin(); qi != queue.end(); ++qi) {
            if (qi->id == id) {
           	    queue.erase(qi);
                break;
            }
        }

        clean_price_level(book, ptr->price);
        index_.erase(it);
        return true;
    }

    std::optional<double> best_bid() const {
        if (bids_.empty()) return std::nullopt;
        return bids_.begin()->first;
    }

    std::optional<double> best_ask() const {
        if (asks_.empty()) return std::nullopt;
        return asks_.begin()->first;
    }

    bool empty() const {
        return bids_.empty() && asks_.empty();
    }
};

} // namespace qf

#endif // ORDERBOOK_SIMULATOR_H
