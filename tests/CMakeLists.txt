#
# Locate test sources
#
FILE (GLOB_RECURSE test_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

#
# Test build rules
#
IF (NOT CMAKE_CROSSCOMPILING)
	FOREACH (T_FILE ${test_SRCS})
		get_filename_component (T_NAME ${T_FILE} NAME_WE)
		add_executable (${T_NAME} ${T_FILE})
		add_test (NAME ${T_NAME} COMMAND ${T_NAME} ${TEST_RUNNER_PARAMS})
		if (LOGGER_ENABLED)
			target_compile_definitions(${T_NAME} PRIVATE ENABLE_LOGGER)
		endif()
		target_link_libraries(${T_NAME} doctest::doctest general spdlog::spdlog ibroker_lib -static-libgcc -static-libstdc++ -static)
	ENDFOREACH (T_FILE ${test_SRCS})
ENDIF (NOT CMAKE_CROSSCOMPILING)
