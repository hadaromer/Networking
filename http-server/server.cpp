#include "server.h"

#pragma comment(lib, "Ws2_32.lib")

HttpServer::HttpServer(int port) : port_(port) {
}

HttpServer::~HttpServer() {
	WSACleanup();
}

void HttpServer::Start() {
	if (Initialize() && CreateListenSocket()) {
		while (true) {
			WaitForActivity();
			HandleSockets();
		}
	}
}

bool HttpServer::Initialize() {
	WSAData wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData) == NO_ERROR;
}

bool HttpServer::CreateListenSocket() {
	listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket_ == INVALID_SOCKET) {
		std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
		return false;
	}

	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(port_);

	if (bind(listenSocket_, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR) {
		std::cerr << "Error at bind(): " << WSAGetLastError() << std::endl;
		closesocket(listenSocket_);
		return false;
	}

	if (listen(listenSocket_, 5) == SOCKET_ERROR) {
		std::cerr << "Error at listen(): " << WSAGetLastError() << std::endl;
		closesocket(listenSocket_);
		return false;
	}

	sockets.resize(MAX_SOCKETS);
	sockets[0].id = listenSocket_;
	sockets[0].recv = LISTEN;

	return true;
}

void HttpServer::WaitForActivity() {
	FD_ZERO(&waitRecv);
	FD_ZERO(&waitSend);

	for (int index = 0; index < sockets.size(); index++) {
		const auto& socket = sockets[index];
		if (socket.recv == RECEIVE && socket.handle == IDLE && isInactive(socket)) {
			closesocket(socket.id);
			RemoveSocket(index);
		}
		if ((socket.recv == LISTEN) || (socket.recv == RECEIVE)) {
			FD_SET(socket.id, &waitRecv);
		}
		if (socket.handle == HANDLE) {
			FD_SET(socket.id, &waitSend);
		}
	}

	select(0, &waitRecv, &waitSend, nullptr, nullptr);
}

void HttpServer::HandleSockets() {
	for (size_t i = 0; i < sockets.size(); ++i) {
		if (FD_ISSET(sockets[i].id, &waitRecv)) {
			switch (sockets[i].recv) {
			case LISTEN:
				AcceptConnection(i);
				break;

			case RECEIVE:
				sockets[i].updateLastActivity();
				ReceiveMessage(i);
				break;
			}
		}
		if (FD_ISSET(sockets[i].id, &waitSend)) {
			if (sockets[i].handle == HANDLE) {
				HandleHttpRequest(i);
			}
		}
	}
}

void HttpServer::AcceptConnection(size_t index) {
	SOCKET id = sockets[index].id;
	sockaddr_in from;
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (msgSocket == INVALID_SOCKET) {
		std::cerr << "Error at accept(): " << WSAGetLastError() << std::endl;
		return;
	}

	std::cout << "Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << std::endl;

	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0) {
		std::cerr << "Error at ioctlsocket(): " << WSAGetLastError() << std::endl;
	}

	if (!AddSocket(msgSocket, from, RECEIVE)) {
		std::cerr << "Too many connections, dropped!" << std::endl;
		closesocket(id);
	}
}

void HttpServer::ReceiveMessage(size_t index) {
	SOCKET msgSocket = sockets[index].id;

	int bytesRecv = recv(msgSocket, sockets[index].buffer, sizeof(sockets[index].buffer), 0);

	if (bytesRecv == SOCKET_ERROR) {
		std::cerr << "Error at recv(): " << WSAGetLastError() << std::endl;
		closesocket(msgSocket);
		RemoveSocket(index);
		return;
	}

	if (bytesRecv == 0) {
		closesocket(msgSocket);
		RemoveSocket(index);
		return;
	}
	else {
		sockets[index].buffer[bytesRecv] = '\0'; //add the null-terminating to make it a string
		std::cout << "Received: " << bytesRecv << " bytes of \"" << sockets[index].buffer << "\" message." << std::endl;

		sockets[index].len += bytesRecv;

		sockets[index].requests.push(Request(sockets[index].buffer));
		sockets[index].handle = HANDLE;
	}
}

void HttpServer::SendMessage(size_t index, Response res, bool shouldAddContent) {
	int bytesSent = 0;
	SOCKET msgSocket = sockets[index].id;

	std::string msg = res.to_string(shouldAddContent);
	bytesSent = send(msgSocket, msg.c_str(), msg.length(), 0);

	if (bytesSent == SOCKET_ERROR) {
		std::cerr << "Error at send(): " << WSAGetLastError() << std::endl;
		return;
	}

	std::cout << "Sent: " << bytesSent << "\\" << msg.length() << " bytes of \"" << msg << "\" message." << std::endl;
}

bool HttpServer::AddSocket(SOCKET id, sockaddr_in from, int what) {
	for (size_t i = 0; i < sockets.size(); ++i) {
		if (sockets[i].recv == EMPTY) {
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].handle = IDLE;
			sockets[i].len = 0;
			sockets[i].ip = std::string(inet_ntoa(from.sin_addr)) + ":" + std::to_string(ntohs(from.sin_port));
			sockets[i].updateLastActivity();
			return true;
		}
	}
	return false;
}

void HttpServer::RemoveSocket(size_t index) {
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	sockets[index].handle = EMPTY;
	sockets[index].name = "";
	sockets[index].lenName = 0;
}

void HttpServer::HandleHttpRequest(size_t index) {
	Request request = sockets[index].requests.front();
	Response res;
	bool shouldAddContent = true;

	int routeStatus = ValidateRoute(request.getPath(), request.getMethod());

	switch (routeStatus) {
	case NOT_ALLOWED:
		res.setStatus(HTTP_METHOD_NOT_ALLOWED);
		break;
	case ROUTE_NOT_FOUND:
		res.setStatus(HTTP_NOT_FOUND);
		break;
	default:
		ProcessValidRoute(index, request, res, shouldAddContent);
	}

	SendMessage(index, res, shouldAddContent);

	sockets[index].requests.pop();
	if (sockets[index].requests.empty()) {
		sockets[index].handle = IDLE;
	}
}

void HttpServer::ProcessValidRoute(size_t index, const Request& request, Response& res, bool& shouldAddContent) {
	if (request.getMethod() == "OPTIONS") {
		HandleOptions(request, res);
	}
	else if (request.getMethod() == "PUT") {
		HandlePut(index, request, res);
	}
	else if (request.getMethod() == "GET" || request.getMethod() == "HEAD") {
		HandleGet(index, request, res);
		if (request.getMethod() == "HEAD") {
			shouldAddContent = false; // in head requests we don't send any data, just headers
		}
	}
	else if (request.getMethod() == "POST") {
		HandlePost(request, res);
	}
	else if (request.getMethod() == "DELETE") {
		HandleDelete(index, res);
	}
	else if (request.getMethod() == "TRACE") {
		HandleTrace(request, res);
	}
	else {
		res.setStatus(HTTP_NOT_IMPLEMENTED);
	}
}

void HttpServer::HandleOptions(const Request& request, Response& res) {
	if (request.getPath() == "*") {
		res.addHeader("Allow", "OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE");
	}
	else {
		auto selectedRoute = routes.find(request.getPath());
		std::string allowedOptions = StringUtils().concatenateWithSpace(selectedRoute->second);
		res.addHeader("Allow", allowedOptions);
	}
}

void HttpServer::HandlePut(size_t index, const Request& request, Response& res) {
	if (sockets[index].lenName == 0) {
		res.setStatus(HTTP_CREATED);
	}
	else {
		res.setStatus(HTTP_OK);
	}
	sockets[index].name = request.getContent()[0];
	sockets[index].lenName = sockets[index].name.length();
}

void HttpServer::HandleGet(size_t index, const Request& request, Response& res) {
	std::string filePath;
	std::string langDir = request.getQueryParam("lang");
	std::string fileName = request.getPath() != "/" ? request.getPath() : "index.html";

	filePath = "html_pages\\" + langDir + "\\" + fileName;

	std::ifstream file(filePath, std::ios::binary);
	if (file.is_open()) {
		std::string content = HandleFileRead(file);
		if (request.getPath() == "/") {
			content = std::regex_replace(content, std::regex("\\{user-data\\}"), getUserData(sockets[index]));
		}
		res.setContent(content);
		res.addHeader("Content-Type", getMimeTypeFromExtension(filePath));
		res.setStatus(HTTP_OK);
	}
	else {
		res.setStatus(HTTP_NOT_FOUND);
	}
}

void HttpServer::HandlePost(const Request& request, Response& res) {
	std::cout << "Client posted: " << std::endl;
	for (const auto& line : request.getContent()) {
		std::cout << line << std::endl;
	}
	res.setStatus(HTTP_OK);
}

void HttpServer::HandleDelete(size_t index, Response& res) {
	sockets[index].name = "";
	sockets[index].lenName = 0;
	res.setStatus(HTTP_OK);
}

void HttpServer::HandleTrace(const Request& request, Response& res) {
	res.addHeader("Content-Type", "message/http");
	res.setStatus(HTTP_OK);
	res.setContent(request.getOriginalRequest());
}

std::string HttpServer::HandleFileRead(std::ifstream& file) {
	std::stringstream fileContent;
	fileContent << file.rdbuf();
	return fileContent.str();
}


int HttpServer::ValidateRoute(const std::string& route, const std::string method) {
	auto selectedRoute = routes.find(route);
	if (selectedRoute == routes.end()) {
		return ROUTE_NOT_FOUND;
	}
	if (std::find(selectedRoute->second.begin(), selectedRoute->second.end(), method) == selectedRoute->second.end()) {
		return NOT_ALLOWED;
	}
	return ALLOWED;
}

std::string HttpServer::getMimeTypeFromExtension(const std::string& filename) {
	size_t pos = filename.find_last_of('.');
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

std::string HttpServer::getUserData(SocketState& socketState) {
	time_t timer;
	time(&timer);
	std::string name = socketState.lenName > 0 ? socketState.name : "guest";
	std::string result = "Hello " + name + "!<br / >Your IP : " + socketState.ip + "<br / >Server time : " + ctime(&timer);

	return result;
}

// Function to check if a socket has been inactive for 2 minutes
bool HttpServer::isInactive(const SocketState& socket) {
	auto currentTime = std::chrono::steady_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - socket.lastActivityTime);
	return elapsedTime >= std::chrono::minutes(2);
}

