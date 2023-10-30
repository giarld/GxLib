cmake_minimum_required(VERSION 3.20)

add_executable(TestGx
        src/test_main.cpp
)

target_link_libraries(TestGx gtest gany-core gx)
