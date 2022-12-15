cmake_minimum_required(VERSION 3.23)
project(ibroker)

set(CMAKE_CXX_STANDARD 14)

# Add dependencies
add_subdirectory(vendor/log-library)
add_subdirectory(vendor/network-library)
add_subdirectory(vendor/aho-corasick-matching)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/log-library)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/network-library/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/aho-corasick-matching/src)

ENABLE_TESTING (true)
add_subdirectory(lib)
add_subdirectory(broker)
add_subdirectory(tests)
