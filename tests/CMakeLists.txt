add_executable(deque_tests 
    test_main.cpp
    "test_deque_buff.cpp"
    "test_deque_iterator.cpp"  
    "test_deque.cpp" 
)

target_link_libraries(deque_tests PRIVATE 
    myDeque 
    GTest::gtest_main
)

target_compile_definitions(deque_tests PRIVATE
    PRINT_DEQUE_ARRAY_2
    PRINT_DEQUE_FIELD_2
)

add_test(NAME deque_tests COMMAND deque_tests)
