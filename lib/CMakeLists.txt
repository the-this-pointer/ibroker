cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(ibroker_lib ConnectionHandler.cpp QueueManager.cpp Queue.cpp)
target_link_libraries(ibroker_lib PUBLIC netLib_static)
