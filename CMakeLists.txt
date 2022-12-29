# some of these lines you see here, are taken from: https://github.com/Lectem/cpp-boilerplate/blob/master/CMakeLists.txt

cmake_minimum_required(VERSION 3.23)
project(ibroker)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS ASIO_STANDALONE WITH_ASIO)

# Custom modules and scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake") # Make our cmake scripts available
include(Warnings)
include(Coverage)

# Use your own option for tests, in case people use your library through add_subdirectory
cmake_dependent_option(${PROJECT_NAME}_BUILD_TESTS
        "Enable ${PROJECT_NAME} project tests targets" ON # By default we want tests if CTest is enabled
        "BUILD_TESTING" OFF # Stay coherent with CTest variables
        )

# Add dependencies
add_subdirectory(vendor EXCLUDE_FROM_ALL)

ENABLE_TESTING (true)
add_subdirectory(lib)
add_subdirectory(broker)
add_subdirectory(tests)

# Again, taken from: https://github.com/Lectem/cpp-boilerplate/blob/master/CMakeLists.txt
#===========#
#   Tests   #
#===========#

if(${PROJECT_NAME}_BUILD_TESTS)
    # Let the user add options to the test runner if needed
    set(TEST_RUNNER_PARAMS "--force-colors=true" CACHE STRING "Options to add to our test runners commands")
    # In a real project you most likely want to exclude test folders
    # list(APPEND CUSTOM_COVERAGE_EXCLUDE "/test/")
    add_subdirectory(tests)
endif()
