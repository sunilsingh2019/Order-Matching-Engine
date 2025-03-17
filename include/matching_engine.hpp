#pragma once
#include "order_book.hpp"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

namespace trading {

class MatchingEngine {
public:
    explicit MatchingEngine(size_t numThreads = std::thread::hardware_concurrency());
    ~MatchingEngine();

    // Non-copyable
    MatchingEngine(const MatchingEngine&) = delete;
    MatchingEngine& operator=(const MatchingEngine&) = delete;

    // Order submission interface
    std::future<bool> submitOrder(std::shared_ptr<Order> order);
    bool cancelOrder(const std::string& orderId);

    // Engine control
    void start();
    void stop();

    // Statistics and monitoring
    double getAverageLatencyMicros() const;
    uint64_t getOrdersProcessedPerSecond() const;

private:
    void processingThread();
    void processOrder(std::shared_ptr<Order> order);
    void handleMarketOrder(std::shared_ptr<Order> order);
    void handleLimitOrder(std::shared_ptr<Order> order);
    void handleStopOrder(std::shared_ptr<Order> order);

    OrderBook orderBook_;
    std::queue<std::shared_ptr<Order>> orderQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::vector<std::thread> workerThreads_;
    std::atomic<bool> running_{false};

    // Performance metrics
    std::atomic<uint64_t> totalLatencyMicros_{0};
    std::atomic<uint64_t> orderCount_{0};
    std::chrono::steady_clock::time_point startTime_;
};