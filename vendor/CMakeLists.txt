project(external C CXX)

set(BUILD_TESTING_BCKP ${BUILD_TESTING})
set(BUILD_TESTING OFF CACHE BOOL "Force disable of tests for external dependencies" FORCE)

set(CUSTOM_COVERAGE_EXCLUDE ${CUSTOM_COVERAGE_EXCLUDE} "external" PARENT_SCOPE) # Replaced in CTestCustom.cmake.in

add_subdirectory(network-library EXCLUDE_FROM_ALL)
target_set_warnings(netLib_static DISABLE ALL)
target_include_directories(
        netLib_static
        INTERFACE
        network-library/inc
)

add_subdirectory(aho-corasick-matching EXCLUDE_FROM_ALL)
target_include_directories(
        aho-corasick-matching
        INTERFACE
        aho-corasick-matching/src
)

add_subdirectory(fmt EXCLUDE_FROM_ALL)
target_set_warnings(fmt DISABLE ALL)

add_subdirectory(spdlog EXCLUDE_FROM_ALL)
target_compile_definitions(spdlog INTERFACE SPDLOG_FMT_EXTERNAL)
target_set_warnings(spdlog DISABLE ALL)

add_library(doctest INTERFACE)
target_include_directories(
        doctest
        INTERFACE
        doctest/doctest # note : will expose the parts/ folder...
)
add_library(doctest::doctest ALIAS doctest)
target_compile_features(doctest INTERFACE cxx_std_11)

set(BUILD_TESTING ${BUILD_TESTING_BCKP} CACHE BOOL "Build tests (default variable for CTest)" FORCE)
