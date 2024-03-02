#include "methods.h"

void HandleOptions(SocketState& socket, const Request& req, Response& res) {
	if (req.getPath() == "*") { // all the allowed options
		res.addHeader("Allow", "OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE");
	}
	else {
		auto selectedRoute = routes.find(req.getPath());
		std::vector<std::string> allowedOptionsArray;
		allowedOptionsArray.reserve(selectedRoute->second.size());

		std::transform(selectedRoute->second.begin(), selectedRoute->second.end(), std::back_inserter(allowedOptionsArray),
			[](const Method& method) { return method.methodName; });
		std::string allowedOptions = StringUtils().concatenateWithSpace(allowedOptionsArray);
		res.addHeader("Allow", allowedOptions);
	}
}

void HandlePut(SocketState& socket, const Request& req, Response& res) {
	if (socket.lenName == 0) {
		res.setStatus(HTTP_CREATED);
	}
	else {
		res.setStatus(HTTP_OK);
	}
	std::vector<std::string> reqContent = req.getContent();
	if (reqContent.size() > 0) {
		socket.name = req.getContent()[0];
		socket.lenName = socket.name.length();
	}
	else {
		res.setStatus(HTTP_BAD_REQUEST);
		res.setContent("Please add your name in request body");
	}
}

void HandleGetName(SocketState& socket, const Request& req, Response& res) {
	if (req.getMethod() == "HEAD") { // HEAD method should return only headers
		res.setShouldIncludeContent(false);
	}
	res.setContent(socket.name);
	res.addHeader("Content-Type", "text/plain");
	res.setStatus(HTTP_OK);
}

void HandleGet(SocketState& socket, const Request& req, Response& res) {
	if (req.getMethod() == "HEAD") { // HEAD method should return only headers
		res.setShouldIncludeContent(false);
	}

	std::string filePath;
	std::string langDir = req.getQueryParam("lang");
	if (langDir == "en") langDir = ""; // english is the default language
	std::string fileName = req.getPath() != "/" ? req.getPath() : "index.html";

	filePath = RESOURCES_PATH + langDir + "\\" + fileName;

	std::ifstream file(filePath, std::ios::binary);
	if (file.is_open()) {
		std::string content = Utils().HandleFileRead(file);
		if (req.getPath() == "/") {
			content = std::regex_replace(content, std::regex("\\{user-data\\}"), Utils().getUserData(socket));
		}
		res.setContent(content);
		res.addHeader("Content-Type", Utils().getMimeTypeFromExtension(filePath));
		res.setStatus(HTTP_OK);
	}

	else {
		res.setStatus(HTTP_NOT_FOUND);
	}
}

void HandlePost(SocketState& socket, const Request& req, Response& res) {
	std::cout << "Client posted: " << std::endl;
	for (const auto& line : req.getContent()) {
		std::cout << line << std::endl;
	}
	res.setStatus(HTTP_OK);
}

void HandleDelete(SocketState& socket, const Request& req, Response& res) {
	socket.name = "";
	socket.lenName = 0;
	res.setStatus(HTTP_OK);
}

void HandleTrace(SocketState& socket, const Request& req, Response& res) {
	res.addHeader("Content-Type", "message/http");
	res.setStatus(HTTP_OK);
	res.setContent(req.getOriginalRequest());
}

