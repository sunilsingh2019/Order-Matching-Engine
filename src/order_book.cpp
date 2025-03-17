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

    if (order->getSide() == OrderSide::BUY) {
        auto& priceLevel = bids_[order->getPrice()];
        priceLevel.price = order->getPrice();
        priceLevel.orders.push_back(order);
    } else {
        auto& priceLevel = asks_[order->getPrice()];
        priceLevel.price = order->getPrice();
        priceLevel.orders.push_back(order);
    }
    
    orderMap_[order->getOrderId()] = order;
    totalOrdersProcessed_++;
    return true;
}

bool OrderBook::cancelOrder(const std::string& orderId) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto it = orderMap_.find(orderId);
    if (it == orderMap_.end()) return false;

    auto order = it->second;
    if (order->getType() == OrderType::STOP) {
        auto range = stopOrders_.equal_range(order->getStopPrice());
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second->getOrderId() == orderId) {
                stopOrders_.erase(it);
                orderMap_.erase(orderId);
                return true;
            }
        }
    }

    if (order->getSide() == OrderSide::BUY) {
        auto levelIt = bids_.find(order->getPrice());
        if (levelIt != bids_.end()) {
            auto& orders = levelIt->second.orders;
            orders.remove_if([&orderId](const auto& o) { 
                return o->getOrderId() == orderId; 
            });
            
            if (orders.empty()) {
                bids_.erase(levelIt);
            }
        }
    } else {
        auto levelIt = asks_.find(order->getPrice());
        if (levelIt != asks_.end()) {
            auto& orders = levelIt->second.orders;
            orders.remove_if([&orderId](const auto& o) { 
                return o->getOrderId() == orderId; 
            });
            
            if (orders.empty()) {
                asks_.erase(levelIt);
            }
        }
    }
    
    orderMap_.erase(orderId);
    return true;
}

bool OrderBook::modifyOrder(const std::string& orderId, double newQuantity) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto it = orderMap_.find(orderId);
    if (it == orderMap_.end()) return false;

    auto order = it->second;
    order->setQuantity(newQuantity);
    return true;
}

std::vector<std::pair<std::shared_ptr<Order>, std::shared_ptr<Order>>> 
OrderBook::matchMarketOrder(std::shared_ptr<Order> order) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    std::vector<std::pair<std::shared_ptr<Order>, std::shared_ptr<Order>>> matches;
    
    double remainingQty = order->getQuantity();
    
    if (order->getSide() == OrderSide::BUY) {
        // For buy orders, match against asks
        for (auto bookIt = asks_.begin(); 
             bookIt != asks_.end() && remainingQty > 0;) {
            // Rest of the matching logic
            auto& priceLevel = bookIt->second;
            
            for (auto orderIt = priceLevel.orders.begin(); 
                 orderIt != priceLevel.orders.end() && remainingQty > 0;) {
                auto matchedOrder = *orderIt;
                double matchQty = std::min(remainingQty, matchedOrder->getQuantity());
                
                matches.emplace_back(order, matchedOrder);
                remainingQty -= matchQty;
                matchedOrder->setQuantity(matchedOrder->getQuantity() - matchQty);
                
                if (matchedOrder->getQuantity() <= 0) {
                    orderMap_.erase(matchedOrder->getOrderId());
                    orderIt = priceLevel.orders.erase(orderIt);
                } else {
                    ++orderIt;
                }
                totalMatchesExecuted_++;
            }
            
            if (priceLevel.orders.empty()) {
                bookIt = asks_.erase(bookIt);
            } else {
                ++bookIt;
            }
        }
    } else {
        // For sell orders, match against bids
        for (auto bookIt = bids_.begin(); 
             bookIt != bids_.end() && remainingQty > 0;) {
            auto& priceLevel = bookIt->second;
            
            for (auto orderIt = priceLevel.orders.begin(); 
                 orderIt != priceLevel.orders.end() && remainingQty > 0;) {
                auto matchedOrder = *orderIt;
                double matchQty = std::min(remainingQty, matchedOrder->getQuantity());
                
                matches.emplace_back(order, matchedOrder);
                remainingQty -= matchQty;
                matchedOrder->setQuantity(matchedOrder->getQuantity() - matchQty);
                
                if (matchedOrder->getQuantity() <= 0) {
                    orderMap_.erase(matchedOrder->getOrderId());
                    orderIt = priceLevel.orders.erase(orderIt);
                } else {
                    ++orderIt;
                }
                totalMatchesExecuted_++;
            }
            
            if (priceLevel.orders.empty()) {
                bookIt = bids_.erase(bookIt);
            } else {
                ++bookIt;
            }
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
    
    for (auto& order : triggeredOrders) {
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

} // namespace trading