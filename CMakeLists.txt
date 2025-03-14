cmake_minimum_required(VERSION 3.29)
project(test_task)

set(CMAKE_CXX_STANDARD 17)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

add_executable(test_task main.cpp)
target_link_libraries(test_task ${CURSES_LIBRARIES})