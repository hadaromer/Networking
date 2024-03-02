#include "StringUtils.h"

std::vector<std::string> StringUtils::split(std::string str) const {
	std::string buffer;
	std::stringstream ss(str);
	std::vector<std::string> result;

	while (ss >> buffer) result.push_back(buffer);

	return result;
}

std::vector<std::string> StringUtils::buffer_to_vector(char* buffer) const {
	std::vector<std::string> lines;
	std::string str = "";
	for (int i = 0; i < std::strlen(buffer); i++) {
		if (buffer[i] == '\n') {
			lines.push_back(str);
			str = "";
		}
		else str += buffer[i];
	}

	if (str.length() > 0) lines.push_back(str);

	return lines;
}

std::vector<std::string> StringUtils::splitString(const std::string& input, char delimiter) const {
	std::vector<std::string> result;
	std::istringstream stream(input);
	std::string token;

	while (std::getline(stream, token, delimiter)) {
		result.push_back(token);
	}

	return result;
}

std::string StringUtils::concatenateWithSpace(const std::vector<std::string>& vec) const {
	std::string result;

	for (const auto& item : vec) {
		result += item + " ";
	}

	// Remove the trailing space
	if (!result.empty()) {
		result.pop_back();
	}

	return result;
}
