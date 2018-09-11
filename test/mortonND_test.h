#pragma once

#include <string>
#include <vector>

struct test_method {
	int(*test_func)();
	std::string description;

	test_method(int(*test_func)(), std::string description) : test_func(test_func), description(description) {}
};