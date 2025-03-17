cmake_minimum_required(VERSION 3.15)
project(OrderMatchingEngine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Main library
add_library(order_matching_engine
    src/order_book.cpp
    src/order.cpp
    src/matching_engine.cpp
)

target_include_directories(order_matching_engine PUBLIC include)

# Main executable
add_executable(trading_engine src/main.cpp)
target_link_libraries(trading_engine PRIVATE order_matching_engine)

# Testing
enable_testing()
add_subdirectory(tests)

# Find and link threading library
find_package(Threads REQUIRED)
target_link_libraries(order_matching_engine PRIVATE Threads::Threads)