#include "order_book.hpp"
#include <algorithm>

namespace trading {

OrderBook::OrderBook() = default;

bool OrderBook::addOrder(std::shared_ptr<Order> order) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    if (order->getType() == OrderType::STOP) {
        stopOrders_.emplace(order->getStopPrice(), order);
        orderMap_[order->getOrderId()] = order;
        return true;
    }

    auto& book = (order->getSide() == OrderSide::BUY) ? bids_ : asks_;
    auto& priceLevel = book[order->getPrice()];
    priceLevel.price = order->getPrice();
    priceLevel.orders.push_back(order);
    orderMap_[order->getOrderId()] = order;
    totalOrdersProcessed_++;
    return true;
}

bool OrderBook::cancelOrder(const std::string& orderId) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto it = orderMap_.find(orderId);
    if (it == orderMap_.end()) return false;

    auto order = it->second;
    auto& book = (order->getSide() == OrderSide::BUY) ? bids_ : asks_;
    
    if (order->getType() == OrderType::STOP) {
        auto range = stopOrders_.equal_range(order->getStopPrice());
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second->getOrderId() == orderId) {
                stopOrders_.erase(it);
                orderMap_.erase(orderId);
                return true;
            }
        }
    } else {
        auto& priceLevel = book[order->getPrice()];
        auto& orders = priceLevel.orders;
        orders.remove_if([&orderId](const auto& o) { return o->getOrderId() == orderId; });
        
        if (orders.empty()) {
            book.erase(order->getPrice());
        }
        orderMap_.erase(orderId);
    }
    return true;
}

std::vector<std::pair<std::shared_ptr<Order>, std::shared_ptr<Order>>> 
OrderBook::matchMarketOrder(std::shared_ptr<Order> order) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    std::vector<std::pair<std::shared_ptr<Order>, std::shared_ptr<Order>>> matches;
    
    auto& oppositeBook = (order->getSide() == OrderSide::BUY) ? asks_ : bids_;
    double remainingQty = order->getQuantity();
    
    while (remainingQty > 0 && !oppositeBook.empty()) {
        auto& bestPrice = (order->getSide() == OrderSide::BUY) ? 
            oppositeBook.begin()->second : oppositeBook.rbegin()->second;
        
        for (auto it = bestPrice.orders.begin(); 
             it != bestPrice.orders.end() && remainingQty > 0;) {
            auto matchedOrder = *it;
            double matchQty = std::min(remainingQty, matchedOrder->getQuantity());
            
            matches.emplace_back(order, matchedOrder);
            remainingQty -= matchQty;
            matchedOrder->setQuantity(matchedOrder->getQuantity() - matchQty);
            
            if (matchedOrder->getQuantity() <= 0) {
                orderMap_.erase(matchedOrder->getOrderId());
                it = bestPrice.orders.erase(it);
            } else {
                ++it;
            }
            totalMatchesExecuted_++;
        }
        
        if (bestPrice.orders.empty()) {
            oppositeBook.erase(bestPrice.price);
        }
    }
    
    return matches;
}

void OrderBook::checkStopOrders(double lastTradePrice) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    std::vector<std::shared_ptr<Order>> triggeredOrders;
    
    auto it = stopOrders_.begin();
    while (it != stopOrders_.end()) {
        auto order = it->second;
        bool shouldTrigger = false;
        
        if (order->getSide() == OrderSide::BUY && lastTradePrice >= order->getStopPrice()) {
            shouldTrigger = true;
        } else if (order->getSide() == OrderSide::SELL && lastTradePrice <= order->getStopPrice()) {
            shouldTrigger = true;
        }
        
        if (shouldTrigger) {
            triggeredOrders.push_back(order);
            it = stopOrders_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Convert triggered stop orders to limit orders
    for (auto& order : triggeredOrders) {
        order->setQuantity(order->getQuantity());  // Convert to limit order at stop price
        addOrder(order);
    }
}

double OrderBook::getBestBid() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return bids_.empty() ? 0.0 : bids_.begin()->first;
}

double OrderBook::getBestAsk() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return asks_.empty() ? 0.0 : asks_.begin()->first;
}