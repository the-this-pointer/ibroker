cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 14)

add_executable(ibroker main.cpp)
target_link_libraries(ibroker PRIVATE ibroker_lib -static-libgcc -static-libstdc++ -static)
