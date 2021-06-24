cmake_minimum_required(VERSION 3.1...3.15)
project(morton-nd-test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -mbmi2")

# TODO: remove once this is fixed.
if (NOT CMAKE_COMPILER_IS_GNUCXX)
	message(FATAL_ERROR "Tests can only be built with GCC.")
endif()

add_executable(morton-nd-test
		main.cpp
		mortonND_BMI2_test.cpp
		mortonND_LUT_test.cpp
		mortonND_test_util.h
		mortonND_test_control.h
		mortonND_test_common.h
		mortonND_test.h
		mortonND_BMI2_test.h
		mortonND_LUT_test.h
		variadic_placeholder.h)

target_link_libraries(morton-nd-test PRIVATE MortonND)

add_test(NAME morton-nd-test COMMAND morton-nd-test)