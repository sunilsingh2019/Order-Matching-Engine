enable_testing()

# Add test executable
add_executable(matching_engine_tests
    order_book_tests.cpp
)

target_link_libraries(matching_engine_tests
    PRIVATE
    order_matching_engine
)

# Add test to CTest
add_test(NAME matching_engine_tests COMMAND matching_engine_tests)