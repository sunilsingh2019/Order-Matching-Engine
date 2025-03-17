#pragma once
#include <string>
#include <chrono>

namespace trading {

enum class OrderType {
    LIMIT,
    MARKET,
    STOP
};

enum class OrderSide {
    BUY,
    SELL
};

class Order {
public:
    Order(const std::string& orderId, 
          OrderType type, 
          OrderSide side,
          double price,
          double quantity,
          double stopPrice = 0.0);

    const std::string& getOrderId() const { return orderId_; }
    OrderType getType() const { return type_; }
    OrderSide getSide() const { return side_; }
    double getPrice() const { return price_; }
    double getQuantity() const { return quantity_; }
    double getStopPrice() const { return stopPrice_; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp_; }

    void setQuantity(double quantity) { quantity_ = quantity; }

private:
    std::string orderId_;
    OrderType type_;
    OrderSide side_;
    double price_;
    double quantity_;
    double stopPrice_;
    std::chrono::system_clock::time_point timestamp_;
};
}