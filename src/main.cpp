#include "matching_engine.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace trading;

int main() {
    // Create matching engine with hardware thread count
    MatchingEngine engine;
    engine.start();

    // Create some sample orders
    auto sellOrder1 = std::make_shared<Order>("sell1", OrderType::LIMIT, OrderSide::SELL, 100.0, 1000);
    auto sellOrder2 = std::make_shared<Order>("sell2", OrderType::LIMIT, OrderSide::SELL, 101.0, 1000);
    auto buyOrder1 = std::make_shared<Order>("buy1", OrderType::LIMIT, OrderSide::BUY, 99.0, 500);
    auto marketBuy = std::make_shared<Order>("mbuy1", OrderType::MARKET, OrderSide::BUY, 0.0, 750);

    // Submit orders
    engine.submitOrder(sellOrder1);
    engine.submitOrder(sellOrder2);
    engine.submitOrder(buyOrder1);
    
    // Small delay to let orders process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Submit market order
    auto result = engine.submitOrder(marketBuy);
    result.wait();

    // Print performance metrics
    std::cout << "Average latency: " << engine.getAverageLatencyMicros() << " microseconds\n";
    std::cout << "Orders/second: " << engine.getOrdersProcessedPerSecond() << "\n";

    engine.stop();
    return 0;
}