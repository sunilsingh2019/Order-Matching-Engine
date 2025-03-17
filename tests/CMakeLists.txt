enable_testing()

# Add test executable
add_executable(order_book_tests
    order_book_tests.cpp
)

target_link_libraries(order_book_tests
    PRIVATE
    order_matching_engine
)

# Add tests to CTest
add_test(
    NAME OrderBookTests
    COMMAND order_book_tests
)

# Set timeout to prevent infinite loops
set_tests_properties(OrderBookTests PROPERTIES TIMEOUT 30)