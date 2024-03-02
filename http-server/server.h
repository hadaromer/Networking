#ifndef SERVER_H
#define SERVER_H
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "http/request.h"
#include "http/response.h"
#include "utils/StringUtils.h"
#include "utils/Utils.h"
#include "common/consts.h"
#include "http/methods.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <regex>


class HttpServer {
public:
	HttpServer(int port);
	~HttpServer();

	void Start();

private:
	int port_;
	SOCKET listenSocket_;
	std::vector<SocketState> sockets;
	fd_set waitRecv, waitSend;

	bool Initialize();
	bool CreateListenSocket();
	void WaitForActivity();
	void HandleSockets();
	void AcceptConnection(int index);
	void ReceiveMessage(int index);
	void SendMessage(int index, Response res);
	bool AddSocket(SOCKET id, sockaddr_in from, int what);
	void RemoveSocket(int index);
	void HandleHttpRequest(int index);
	void ProcessValidRoute(int index, const Request& request, Response& res, const Method& method);
	int ValidateRoute(const std::string& route, const std::string method, Method& outMethod);
	bool isInactive(const SocketState& socket);
};

#endif //SERVER_H