#include "matching_engine.hpp"
#include <chrono>

namespace trading {

MatchingEngine::MatchingEngine(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workerThreads_.emplace_back(&MatchingEngine::processingThread, this);
    }
    startTime_ = std::chrono::steady_clock::now();
}

MatchingEngine::~MatchingEngine() {
    stop();
    for (auto& thread : workerThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void MatchingEngine::start() {
    running_ = true;
    startTime_ = std::chrono::steady_clock::now();
}

void MatchingEngine::stop() {
    running_ = false;
    queueCV_.notify_all();
}

std::future<bool> MatchingEngine::submitOrder(std::shared_ptr<Order> order) {
    auto promise = std::make_shared<std::promise<bool>>();
    auto future = promise->get_future();
    
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        orderQueue_.push(order);
    }
    queueCV_.notify_one();
    
    return future;
}

bool MatchingEngine::cancelOrder(const std::string& orderId) {
    return orderBook_.cancelOrder(orderId);
}

void MatchingEngine::processingThread() {
    while (running_) {
        std::shared_ptr<Order> order;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] { 
                return !orderQueue_.empty() || !running_; 
            });
            
            if (!running_) break;
            
            order = orderQueue_.front();
            orderQueue_.pop();
        }
        
        if (order) {
            auto start = std::chrono::high_resolution_clock::now();
            processOrder(order);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            totalLatencyMicros_ += latency.count();
            orderCount_++;
        }
    }
}

void MatchingEngine::processOrder(std::shared_ptr<Order> order) {
    switch (order->getType()) {
        case OrderType::MARKET:
            handleMarketOrder(order);
            break;
        case OrderType::LIMIT:
            handleLimitOrder(order);
            break;
        case OrderType::STOP:
            handleStopOrder(order);
            break;
    }
}

void MatchingEngine::handleMarketOrder(std::shared_ptr<Order> order) {
    auto matches = orderBook_.matchMarketOrder(order);
    // Process matches - in production would notify trading parties
    if (!matches.empty()) {
        double lastPrice = matches.back().second->getPrice();
        orderBook_.checkStopOrders(lastPrice);
    }
}

void MatchingEngine::handleLimitOrder(std::shared_ptr<Order> order) {
    auto matches = orderBook_.matchMarketOrder(order);
    if (!matches.empty()) {
        double lastPrice = matches.back().second->getPrice();
        orderBook_.checkStopOrders(lastPrice);
    }
    
    if (order->getQuantity() > 0) {
        orderBook_.addOrder(order);
    }
}

void MatchingEngine::handleStopOrder(std::shared_ptr<Order> order) {
    orderBook_.addOrder(order);
}

double MatchingEngine::getAverageLatencyMicros() const {
    uint64_t count = orderCount_;
    return count > 0 ? static_cast<double>(totalLatencyMicros_) / count : 0.0;
}

uint64_t MatchingEngine::getOrdersProcessedPerSecond() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime_).count();
    return duration > 0 ? orderCount_ / duration : 0;
}

} // namespace trading