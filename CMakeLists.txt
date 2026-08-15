cmake_minimum_required(VERSION 3.15)
project(forex_arbitrage LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(forex_arbitrage
    src/main.cpp
    src/Graph.cpp
    src/RateFetcher.cpp
)

target_include_directories(forex_arbitrage PRIVATE src)

find_package(CURL REQUIRED)
target_link_libraries(forex_arbitrage PRIVATE CURL::libcurl)
