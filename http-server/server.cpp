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
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		std::cout << "Time Server: Error at WSAStartup()\n";
		return false;
	}
	return true;
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
		if (socket.recv == RECEIVE && socket.send == IDLE && isInactive(socket)) {
			closesocket(socket.id);
			RemoveSocket(index);
		}
		if ((socket.recv == LISTEN) || (socket.recv == RECEIVE)) {
			FD_SET(socket.id, &waitRecv);
		}
		if (socket.send == SEND) {
			FD_SET(socket.id, &waitSend);
		}
	}

	select(0, &waitRecv, &waitSend, nullptr, &SELECT_TIMEOUT);
}

void HttpServer::HandleSockets() {
	for (int i = 0; i < sockets.size(); ++i) {
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
			if (sockets[i].send == SEND) {
				HandleHttpRequest(i);
			}
		}
	}
}

void HttpServer::AcceptConnection(int index) {
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

void HttpServer::ReceiveMessage(int index) {
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

		sockets[index].len = bytesRecv;

		sockets[index].requests.push(Request(sockets[index].buffer));
		sockets[index].send = SEND;
	}
}

void HttpServer::SendMessage(int index, Response res) {
	int bytesSent = 0;
	SOCKET msgSocket = sockets[index].id;

	std::string msg = res.to_string();
	bytesSent = send(msgSocket, msg.c_str(), msg.length(), 0);

	if (bytesSent == SOCKET_ERROR) {
		std::cerr << "Error at send(): " << WSAGetLastError() << std::endl;
		return;
	}

	std::cout << "Sent: " << bytesSent << "\\" << msg.length() << " bytes of \"" << msg << "\" message." << std::endl;
}

bool HttpServer::AddSocket(SOCKET id, sockaddr_in from, int what) {
	for (int i = 0; i < sockets.size(); ++i) {
		if (sockets[i].recv == EMPTY) {
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			sockets[i].ip = std::string(inet_ntoa(from.sin_addr)) + ":" + std::to_string(ntohs(from.sin_port));
			sockets[i].updateLastActivity();
			return true;
		}
	}
	return false;
}

void HttpServer::RemoveSocket(int index) {
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	sockets[index].name = "";
	sockets[index].lenName = 0;
}

void HttpServer::HandleHttpRequest(int index) {
	Request request = sockets[index].requests.front();
	Response res;
	Method currentMethod;
	int routeStatus = ValidateRoute(request.getPath(), request.getMethod(), currentMethod);

	switch (routeStatus) {
	case NOT_ALLOWED:
		res.setStatus(HTTP_METHOD_NOT_ALLOWED);
		break;
	case ROUTE_NOT_FOUND:
		res.setStatus(HTTP_NOT_FOUND);
		break;
	default:
		ProcessValidRoute(index, request, res, currentMethod);
	}

	SendMessage(index, res);

	sockets[index].requests.pop();
	if (sockets[index].requests.empty()) {
		sockets[index].send = IDLE;
	}
}

void HttpServer::ProcessValidRoute(int index, const Request& request, Response& res, const Method& method) {
	method.handleFunction(sockets[index], request, res);
}

int HttpServer::ValidateRoute(const std::string& route, const std::string method, Method& outMethod) {
	auto selectedRoute = routes.find(route);
	if (selectedRoute == routes.end()) {
		return ROUTE_NOT_FOUND;
	}
	auto selectedMethod = std::find_if(selectedRoute->second.begin(), selectedRoute->second.end(),
		[&cm = method]
	(const Method& m) -> bool {return m.methodName == cm; });
	if (selectedMethod == selectedRoute->second.end()) {
		return NOT_ALLOWED;
	}
	outMethod = selectedMethod[0];
	return ALLOWED;
}

// Function to check if a socket has been inactive for 2 minutes
bool HttpServer::isInactive(const SocketState& socket) {
	auto currentTime = std::chrono::steady_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - socket.lastActivityTime);
	return elapsedTime >= std::chrono::minutes(2);
}