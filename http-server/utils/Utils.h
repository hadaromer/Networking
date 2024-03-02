#ifndef UTILS_H
#define UTILS_H

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include "../common/consts.h"

const std::unordered_map<std::string, std::string> mime_types = {
{"txt", "text/plain"},
{"html", "text/html"},
{"jpg", "image/jpeg"},
};

class Utils {

public:
	std::string HandleFileRead(std::ifstream& file);
	std::string getMimeTypeFromExtension(const std::string& filename);
	std::string getCurrentDate();
	std::string getUserData(SocketState& socketState);
};

#endif //UTILS_H