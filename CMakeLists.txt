cmake_minimum_required(VERSION 3.23)
project(ibroker)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS ASIO_STANDALONE WITH_ASIO)

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
