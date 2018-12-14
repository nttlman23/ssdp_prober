cmake_minimum_required(VERSION 2.8)
project(ssdps)

#server
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES_SERVER server.cpp)

add_executable(ssdps ${SOURCE_FILES_SERVER})

#client
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES_CLIENT client.cpp)

add_executable(ssdpc ${SOURCE_FILES_CLIENT})

