cmake_minimum_required(VERSION 2.8)
project(ssdp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(ssdp ${SOURCE_FILES})

target_link_libraries(ssdp)
