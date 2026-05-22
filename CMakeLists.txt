cmake_minimum_required(VERSION 3.10)
project(ssdp LANGUAGES CXX)

# Require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(APPLE)
    # macOS specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework SystemConfiguration")
elseif(WIN32)
    # Windows needs winsock library
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lws2_32")
else()
    # Linux/Unix common flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Main executable
add_executable(ssdp main.cpp)

# Set output directory
set_target_properties(ssdp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
