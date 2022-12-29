cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
FILE (GLOB_RECURSE srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(ibroker_lib ${srcs})

if (LOGGER_ENABLED)
    target_compile_definitions(ibroker_lib PRIVATE ENABLE_LOGGER)
endif()

target_link_libraries(ibroker_lib PUBLIC general spdlog::spdlog netLib_static aho-corasick-matching)
