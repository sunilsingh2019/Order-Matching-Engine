#include "order_book.hpp"
#include <algorithm>

using namespace std;

namespace trading {

OrderBook::OrderBook() = default;

bool OrderBook::addOrder(shared_ptr<Order> order) {
    unique_lock<shared_mutex> lock(mutex_);
    
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

bool OrderBook::cancelOrder(const string& orderId) {
    unique_lock<shared_mutex> lock(mutex_);
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

    auto& book = (order->getSide() == OrderSide::BUY) ? bids_ : asks_;
    auto levelIt = book.find(order->getPrice());
    if (levelIt != book.end()) {
        auto& orders = levelIt->second.orders;
        orders.remove_if([&orderId](const auto& o) { 
            return o->getOrderId() == orderId; 
        });
        
        if (orders.empty()) {
            book.erase(levelIt);
        }
    }
    orderMap_.erase(orderId);
    return true;
}

vector<pair<shared_ptr<Order>, shared_ptr<Order>>> 
OrderBook::matchMarketOrder(shared_ptr<Order> order) {
    unique_lock<shared_mutex> lock(mutex_);
    vector<pair<shared_ptr<Order>, shared_ptr<Order>>> matches;
    
    auto& oppositeBook = (order->getSide() == OrderSide::BUY) ? asks_ : bids_;
    double remainingQty = order->getQuantity();
    
    for (auto bookIt = oppositeBook.begin(); 
         bookIt != oppositeBook.end() && remainingQty > 0;) {
        auto& priceLevel = bookIt->second;
        
        for (auto orderIt = priceLevel.orders.begin(); 
             orderIt != priceLevel.orders.end() && remainingQty > 0;) {
            auto matchedOrder = *orderIt;
            double matchQty = min(remainingQty, matchedOrder->getQuantity());
            
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
            bookIt = oppositeBook.erase(bookIt);
        } else {
            ++bookIt;
        }
    }
    
    return matches;
}

void OrderBook::checkStopOrders(double lastTradePrice) {
    unique_lock<shared_mutex> lock(mutex_);
    vector<shared_ptr<Order>> triggeredOrders;
    
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
    shared_lock<shared_mutex> lock(mutex_);
    return bids_.empty() ? 0.0 : bids_.begin()->first;
}

double OrderBook::getBestAsk() const {
    shared_lock<shared_mutex> lock(mutex_);
    return asks_.empty() ? 0.0 : asks_.begin()->first;
}

} // namespace trading