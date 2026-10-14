# 1. Создаем исполняемый файл с уникальным именем deque_tests
add_executable(deque_tests 
    test_main.cpp
    "test_deque_buff.cpp"
    "test_deque_iterator.cpp"  
    "test_deque.cpp" 
)
# 2. Напрямую говорим тестам, где искать "gtest/gtest.hpp"
target_include_directories(deque_tests PRIVATE
    "C:/CppProjectsMSVS/googletest-1.18.0/googletest/include"
)
# 3. Связываем тесты с вашим контейнером и с GoogleTest
target_link_libraries(deque_tests PRIVATE 
    myDeque 
    gtest_main
)
# 4. Передаем макросы логирования
target_compile_definitions(deque_tests PRIVATE
    PRINT_DEQUE_ARRAY_2
    PRINT_DEQUE_FIELD_2
)
# 5. Регистрируем тесты в системе
include(GoogleTest)
gtest_discover_tests(deque_tests)