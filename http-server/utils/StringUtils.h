#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <cstring>

class StringUtils {

public:
	std::vector<std::string> split(std::string str) const;
	std::vector<std::string> buffer_to_vector(char* buffer) const;
	std::vector<std::string> splitString(const std::string& input, char delimiter) const;
	std::string concatenateWithSpace(const std::vector<std::string>& vec) const;
};

#endif //STRING_UTILS_H