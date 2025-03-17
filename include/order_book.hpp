#pragma once
#include "order.hpp"
#include <map>
#include <unordered_map>
#include <list>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <type_traits>
#include <utility>

namespace trading {

using std::map;
using std::unordered_map;
using std::list;
using std::shared_ptr;
using std::pair;
using std::vector;
using std::shared_mutex;
using std::unique_lock;
using std::shared_lock;
using std::atomic;
using std::multimap;
using std::greater;

// Forward declaration
class MatchingEngine;

struct PriceLevel {
    double price;
    list<shared_ptr<Order>> orders;
};

class OrderBook {
public:
    OrderBook();

    // Thread-safe order operations
    bool addOrder(shared_ptr<Order> order);
    bool cancelOrder(const std::string& orderId);
    bool modifyOrder(const std::string& orderId, double newQuantity);

    // Market data queries
    double getBestBid() const;
    double getBestAsk() const;
    
    // Matching operations
    vector<pair<shared_ptr<Order>, shared_ptr<Order>>> 
    matchMarketOrder(shared_ptr<Order> order);
    
    // Stop order processing
    void checkStopOrders(double lastTradePrice);

private:
    // Price-ordered maps for bids and asks
    map<double, PriceLevel, greater<double>> bids_;  // Descending for bids
    map<double, PriceLevel> asks_;                   // Ascending for asks
    
    // Quick order lookup
    unordered_map<std::string, shared_ptr<Order>> orderMap_;
    
    // Stop orders waiting to be triggered
    multimap<double, shared_ptr<Order>> stopOrders_;
    
    // Thread synchronization
    mutable shared_mutex mutex_;
    
    // Performance metrics
    atomic<uint64_t> totalOrdersProcessed_{0};
    atomic<uint64_t> totalMatchesExecuted_{0};
    
    friend class MatchingEngine;
};