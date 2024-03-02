#include "Utils.h"

std::string Utils::HandleFileRead(std::ifstream& file) {
	std::stringstream fileContent;
	fileContent << file.rdbuf();
	return fileContent.str();
}

std::string Utils::getMimeTypeFromExtension(const std::string& filename) {
	int pos = filename.find_last_of('.');
	if (pos == std::string::npos) {
		return ""; // No extension found
	}
	std::string extension = filename.substr(pos + 1);
	auto it = mime_types.find(extension);
	if (it == mime_types.end()) {
		return ""; // Unknown extension
	}
	return it->second;
}

std::string Utils::getCurrentDate() {
	time_t timer;
	time(&timer);
	std::string date = ctime(&timer);
	return date;
}

std::string Utils::getUserData(SocketState& socketState) {
	time_t timer;
	time(&timer);
	std::string name = socketState.lenName > 0 ? socketState.name : "guest";
	std::string result = "Hello " + name + "!<br / >Your IP : " + socketState.ip + "<br / >Server time : " + ctime(&timer);

	return result;
}