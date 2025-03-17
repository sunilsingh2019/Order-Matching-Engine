#pragma once
#include "order.hpp"
#include <map>
#include <unordered_map>
#include <list>
#include <memory>
#include <shared_mutex>
#include <atomic>

namespace trading {

// Forward declaration
class MatchingEngine;

struct PriceLevel {
    double price;
    std::list<std::shared_ptr<Order>> orders;
};

class OrderBook {
public:
    OrderBook();

    // Thread-safe order operations
    bool addOrder(std::shared_ptr<Order> order);
    bool cancelOrder(const std::string& orderId);
    bool modifyOrder(const std::string& orderId, double newQuantity);

    // Market data queries
    double getBestBid() const;
    double getBestAsk() const;
    
    // Matching operations
    std::vector<std::pair<std::shared_ptr<Order>, std::shared_ptr<Order>>> 
    matchMarketOrder(std::shared_ptr<Order> order);
    
    // Stop order processing
    void checkStopOrders(double lastTradePrice);

private:
    // Price-ordered maps for bids and asks
    std::map<double, PriceLevel, std::greater<double>> bids_;  // Descending for bids
    std::map<double, PriceLevel> asks_;                        // Ascending for asks
    
    // Quick order lookup
    std::unordered_map<std::string, std::shared_ptr<Order>> orderMap_;
    
    // Stop orders waiting to be triggered
    std::multimap<double, std::shared_ptr<Order>> stopOrders_;
    
    // Thread synchronization
    mutable std::shared_mutex mutex_;
    
    // Performance metrics
    std::atomic<uint64_t> totalOrdersProcessed_{0};
    std::atomic<uint64_t> totalMatchesExecuted_{0};
    
    friend class MatchingEngine;
};