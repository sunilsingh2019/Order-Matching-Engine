#include "order.hpp"

namespace trading {

Order::Order(const std::string& orderId,
            OrderType type,
            OrderSide side,
            double price,
            double quantity,
            double stopPrice)
    : orderId_(orderId)
    , type_(type)
    , side_(side)
    , price_(price)
    , quantity_(quantity)
    , stopPrice_(stopPrice)
    , timestamp_(std::chrono::system_clock::now())
{
}