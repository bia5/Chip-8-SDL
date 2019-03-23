#pragma once

#include <string>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>

class SplitString {
public:

	SplitString();

	std::vector<std::string> split(std::string, std::string);
	bool isInteger(const std::string & s);
};