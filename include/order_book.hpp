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

class MatchingEngine;

struct PriceLevel {
    double price;
    std::list<std::shared_ptr<Order>> orders;
};

class OrderBook {
public:
    OrderBook();

    bool addOrder(std::shared_ptr<Order> order);
    bool cancelOrder(const std::string& orderId);
    bool modifyOrder(const std::string& orderId, double newQuantity);

    double getBestBid() const;
    double getBestAsk() const;
    
    std::vector<std::pair<std::shared_ptr<Order>, std::shared_ptr<Order>>> 
    matchMarketOrder(std::shared_ptr<Order> order);
    
    void checkStopOrders(double lastTradePrice);

private:
    std::map<double, PriceLevel, std::greater<double>> bids_;
    std::map<double, PriceLevel> asks_;
    std::unordered_map<std::string, std::shared_ptr<Order>> orderMap_;
    std::multimap<double, std::shared_ptr<Order>> stopOrders_;
    mutable std::shared_mutex mutex_;
    std::atomic<uint64_t> totalOrdersProcessed_{0};
    std::atomic<uint64_t> totalMatchesExecuted_{0};
    
    friend class MatchingEngine;
};