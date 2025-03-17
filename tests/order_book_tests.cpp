#include "../include/matching_engine.hpp"
#include "../include/order_book.hpp"
#include <cassert>
#include <iostream>
#include <memory>

using namespace trading;

void testLimitOrderMatching() {
    OrderBook book;
    
    // Create a sell limit order
    auto sellOrder = std::make_shared<Order>("sell1", OrderType::LIMIT, OrderSide::SELL, 100.0, 10);
    book.addOrder(sellOrder);
    
    // Create a matching buy limit order
    auto buyOrder = std::make_shared<Order>("buy1", OrderType::LIMIT, OrderSide::BUY, 100.0, 5);
    auto matches = book.matchMarketOrder(buyOrder);
    
    assert(matches.size() == 1);
    assert(matches[0].first->getOrderId() == "buy1");
    assert(matches[0].second->getOrderId() == "sell1");
    assert(sellOrder->getQuantity() == 5);  // Should have 5 remaining
    
    std::cout << "Limit order matching test passed\n";
}

void testMarketOrderMatching() {
    OrderBook book;
    
    // Add some limit orders to the book
    auto sell1 = std::make_shared<Order>("sell1", OrderType::LIMIT, OrderSide::SELL, 100.0, 10);
    auto sell2 = std::make_shared<Order>("sell2", OrderType::LIMIT, OrderSide::SELL, 101.0, 10);
    book.addOrder(sell1);
    book.addOrder(sell2);
    
    // Create a market buy order
    auto buyMarket = std::make_shared<Order>("buy1", OrderType::MARKET, OrderSide::BUY, 0.0, 15);
    auto matches = book.matchMarketOrder(buyMarket);
    
    assert(matches.size() == 2);
    assert(matches[0].second->getPrice() == 100.0);
    assert(matches[1].second->getPrice() == 101.0);
    
    std::cout << "Market order matching test passed\n";
}

void testStopOrderTrigger() {
    OrderBook book;
    
    // Add a stop order
    auto stopOrder = std::make_shared<Order>("stop1", OrderType::STOP, OrderSide::SELL, 95.0, 10, 100.0);
    book.addOrder(stopOrder);
    
    // Simulate price movement
    book.checkStopOrders(101.0);  // This should trigger the stop order
    
    assert(book.getBestAsk() == 95.0);  // The stop order should now be a limit order
    
    std::cout << "Stop order trigger test passed\n";
}

void testMultiLevelOrderBook() {
    OrderBook book;
    
    // Add multiple price levels
    book.addOrder(std::make_shared<Order>("sell1", OrderType::LIMIT, OrderSide::SELL, 100.0, 10));
    book.addOrder(std::make_shared<Order>("sell2", OrderType::LIMIT, OrderSide::SELL, 101.0, 10));
    book.addOrder(std::make_shared<Order>("buy1", OrderType::LIMIT, OrderSide::BUY, 99.0, 10));
    book.addOrder(std::make_shared<Order>("buy2", OrderType::LIMIT, OrderSide::BUY, 98.0, 10));
    
    assert(book.getBestBid() == 99.0);
    assert(book.getBestAsk() == 100.0);
    
    std::cout << "Multi-level order book test passed\n";
}

int main() {
    try {
        testLimitOrderMatching();
        testMarketOrderMatching();
        testStopOrderTrigger();
        testMultiLevelOrderBook();
        
        std::cout << "All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}