cmake_minimum_required(VERSION 3.14)

project(demo LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(demo
    demo.cpp
)


target_link_libraries(demo -lncurses)
target_link_libraries(demo -lcdk)
