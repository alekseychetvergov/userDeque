cmake_minimum_required(VERSION 3.20)
project(myDeque_Final CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Отключаем предупреждения gtest в Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# 2. Подключаем КОРЕНЬ папки GoogleTest (где лежит её CMakeLists.txt)
add_subdirectory(
    "C:/CppProjectsMSVS/googletest-1.18.0" 
    "${CMAKE_BINARY_DIR}/googletest_build"
)

# 3. Создаем заголовочную библиотеку из вашего контейнера
add_library(myDeque INTERFACE)

# 4. Указываем папки include вашего проекта
target_include_directories(myDeque INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 5. Подключаем подпапку с тестами
add_subdirectory(tests)
