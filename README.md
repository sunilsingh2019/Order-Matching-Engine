# High-Performance Order Matching Engine

A high-performance order matching engine implemented in C++ that supports multiple order types, manages order books efficiently, and processes trades with ultra-low latency. The system implements price-time priority matching algorithms and supports parallel order processing.

## Features

- **Multiple Order Types Support**
  - Limit Orders: Orders with specific price points
  - Market Orders: Orders executed at best available price
  - Stop Orders: Triggered when market hits specified price

- **High-Performance Design**
  - Lock-free data structures for concurrent operations
  - Optimized order book implementation using std::map with custom comparators
  - Efficient memory management
  - Thread-safe order processing

- **Advanced Order Book Management**
  - Price-time priority matching
  - Bid/Ask spread tracking
  - Real-time order book updates
  - Efficient order cancellation and modification

- **Performance Metrics**
  - Real-time latency monitoring
  - Orders processed per second tracking
  - Match execution statistics
  - Performance profiling capabilities

## Technical Architecture

### Core Components

1. **Order Book (OrderBook)**
   - Maintains separate books for bids and asks
   - Implements price-time priority
   - Provides O(log n) lookup for price levels
   - Thread-safe operations using shared mutexes

2. **Matching Engine (MatchingEngine)**
   - Handles order matching logic
   - Processes different order types
   - Manages concurrent order execution
   - Monitors stop orders and triggers

3. **Order Management**
   - Supports order creation, modification, and cancellation
   - Handles multiple order types
   - Maintains order state and history

## Building and Running

### Prerequisites

- CMake (version 3.15 or higher)
- C++17 compliant compiler
- Threading support

### Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Generate build files
cmake ..

# Build the project
make

# Run tests
make test
```

### Running the Engine

```bash
# From the build directory
./trading_engine
```

## Performance Optimization

The engine is optimized for high-performance trading:

- Compiler optimizations (-O3 -march=native)
- Lock-free data structures where possible
- Efficient memory management
- Optimized data structures for order book operations

## Testing

The project includes comprehensive unit tests covering:

- Order book operations
- Different order type handling
- Edge cases and error conditions
- Performance benchmarks

Tests are implemented using modern C++ testing frameworks and can be run using CTest.

## Project Structure

```
.
├── CMakeLists.txt           # Main CMake configuration
├── include/                 # Header files
│   ├── matching_engine.hpp
│   ├── order_book.hpp
│   └── order.hpp
├── src/                    # Source files
│   ├── main.cpp
│   ├── matching_engine.cpp
│   ├── order_book.cpp
│   └── order.cpp
└── tests/                  # Test files
    ├── CMakeLists.txt
    └── order_book_tests.cpp
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests with improvements or bug fixes.

## License

This project is open source and available under the MIT License.

## Future Enhancements

- Market data feed integration
- Additional order types support
- FIX protocol implementation
- Advanced analytics and reporting
- GUI for order book visualization
- Network interface for order submission

## Performance Metrics

The engine is designed to handle:
- Low microsecond-level latency
- High throughput order processing
- Concurrent order execution
- Real-time market data updates

## Development Guide

### Setting Up Development Environment

1. **Clone the Repository**
   ```bash
   git clone <repository-url>
   cd Order-Matching-Engine
   ```

2. **Install Dependencies**
   - Ensure you have a modern C++ compiler (GCC 7+, Clang 6+, or MSVC 2019+)
   - Install CMake 3.15 or higher
   - Optional: Install a debugger (GDB/LLDB)

3. **Build Configuration**
   The project uses the following compiler flags for optimization:
   - `-O3`: Maximum optimization
   - `-march=native`: CPU-specific optimizations
   
   These can be adjusted in `CMakeLists.txt` based on your needs.

### Understanding the Codebase

1. **Order Types (`include/order.hpp`)**
   - `LIMIT`: Orders with specific price points
   - `MARKET`: Orders executed at best available price
   - `STOP`: Orders triggered at specific price points

2. **Order Book Implementation (`include/order_book.hpp`)**
   - Uses `std::map` with custom comparators for price levels
   - Thread-safe operations with `std::shared_mutex`
   - Separate books for bids (descending order) and asks (ascending order)

3. **Matching Engine (`include/matching_engine.hpp`)**
   - Handles incoming orders
   - Implements matching logic
   - Manages concurrent order processing

### Common Development Tasks

1. **Adding a New Order Type**
   - Add enum in `include/order.hpp`
   - Implement handling in `OrderBook::addOrder()`
   - Add matching logic in `MatchingEngine`
   - Update tests

2. **Performance Optimization**
   - Use profiling tools (e.g., perf, valgrind)
   - Monitor latency metrics
   - Check lock contention
   - Analyze memory allocation patterns

3. **Testing**
   - Run unit tests: `make test`
   - Add new test cases in `tests/order_book_tests.cpp`
   - Test different market scenarios
   - Verify thread safety

### Debugging

1. **Common Debug Tools**
   ```bash
   # Run with GDB
   gdb ./trading_engine
   
   # Memory analysis
   valgrind --tool=memcheck ./trading_engine
   ```

2. **Performance Analysis**
   ```bash
   # CPU profiling
   perf record ./trading_engine
   perf report
   ```

### Best Practices

1. **Code Style**
   - Follow modern C++ guidelines
   - Use RAII principles
   - Prefer references to pointers
   - Use const correctness

2. **Thread Safety**
   - Use RAII lock guards
   - Minimize critical sections
   - Avoid nested locks
   - Document thread safety assumptions

3. **Error Handling**
   - Use exceptions for exceptional cases
   - Return values for expected failures
   - Log important events
   - Maintain audit trail

## Acknowledgments

This project was developed as a high-performance trading system implementation, incorporating modern C++ features and best practices for financial trading systems.