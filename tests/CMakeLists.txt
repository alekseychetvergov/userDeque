# 1. Создаем исполняемый файл с уникальным именем deque_tests
add_executable(deque_tests 
    test_main.cpp
    "test_deque_buff.cpp"
    "test_deque_iterator.cpp"  
    "test_deque.cpp" 
)
# 2. Напрямую говорим тестам, где искать "gtest/gtest.hpp"
# 3. Связываем тесты с вашим контейнером и с GoogleTest
target_link_libraries(deque_tests PRIVATE 
    myDeque 
    GTest::gtest_main
)
# 4. Передаем макросы логирования
target_compile_definitions(deque_tests PRIVATE
    PRINT_DEQUE_ARRAY_2
    PRINT_DEQUE_FIELD_2
)
# 5. Регистрируем тесты в системе
add_test(NAME deque_tests COMMAND deque_tests)
