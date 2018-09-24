cmake_minimum_required(VERSION 3.9)
project(morton-nd)

set(CMAKE_CXX_STANDARD 14)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-steps=2147483647")
endif()

add_executable(morton-nd
		main.cpp
		mortonND_LUT_encoder_test.cpp
		mortonND_test_util.h
		mortonND_test_control.h
		mortonND_test_common.h
		mortonND_test.h
		mortonND_LUT_encoder_test.h
		variadic_placeholder.h
		../morton-nd/include/mortonND_LUT_encoder.h
		../morton-nd/include/mortonND_BMI2.h)
