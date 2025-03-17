#include "matching_engine.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <thread>

using namespace trading;
using namespace std;

// Helper function to generate random order IDs
string generateOrderId() {
    static atomic<uint64_t> orderId{0};
    return "order_" + to_string(++orderId);
}

// Helper function to generate random prices around a base price
double generatePrice(double basePrice, double variance, mt19937& gen) {
    uniform_real_distribution<> dis(-variance, variance);
    return basePrice + dis(gen);
}

int main() {
    // Create the matching engine with hardware thread count
    MatchingEngine engine;
    engine.start();

    // Random number generation
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> quantityDis(1.0, 100.0);
    uniform_int_distribution<> sideDis(0, 1);

    const double basePrice = 100.0;
    const double variance = 2.0;
    
    cout << "Starting order matching engine simulation...\n";
    cout << "Submitting orders...\n";

    // Submit some limit orders to build the book
    for (int i = 0; i < 10; ++i) {
        auto side = sideDis(gen) == 0 ? OrderSide::BUY : OrderSide::SELL;
        double price = generatePrice(basePrice, variance, gen);
        double quantity = quantityDis(gen);

        auto order = make_shared<Order>(
            generateOrderId(),
            OrderType::LIMIT,
            side,
            price,
            quantity
        );

        auto result = engine.submitOrder(order);
        result.wait();
        
        cout << "Submitted " << (side == OrderSide::BUY ? "BUY" : "SELL")
             << " order: Price=" << fixed << setprecision(2) << price
             << " Qty=" << quantity << "\n";
    }

    // Submit a market order
    auto marketOrder = make_shared<Order>(
        generateOrderId(),
        OrderType::MARKET,
        OrderSide::BUY,
        0.0,  // Price is ignored for market orders
        50.0
    );

    cout << "\nSubmitting market order...\n";
    auto result = engine.submitOrder(marketOrder);
    result.wait();

    // Submit a stop order
    auto stopOrder = make_shared<Order>(
        generateOrderId(),
        OrderType::STOP,
        OrderSide::SELL,
        basePrice - variance,  // Limit price
        100.0,
        basePrice + variance   // Stop price
    );

    cout << "Submitting stop order...\n";
    result = engine.submitOrder(stopOrder);
    result.wait();

    // Let the engine process orders
    this_thread::sleep_for(chrono::seconds(1));

    // Print performance metrics
    cout << "\nPerformance Metrics:\n";
    cout << "Average latency: " << engine.getAverageLatencyMicros() << " microseconds\n";
    cout << "Orders/second: " << engine.getOrdersProcessedPerSecond() << "\n";

    engine.stop();
    return 0;
}