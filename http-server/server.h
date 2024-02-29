#ifndef SERVER_H
#define SERVER_H
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "http/request.h"
#include "http/response.h"
#include "utils/StringUtils.h"
#include "utils/Utils.h"
#include "common/consts.h"

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
	void AcceptConnection(size_t index);
	void ReceiveMessage(size_t index);
	void SendMessage(size_t index, Response res, bool shouldAddContent);
	bool AddSocket(SOCKET id, sockaddr_in from, int what);
	void RemoveSocket(size_t index);
	void HandleHttpRequest(size_t index);
	void ProcessValidRoute(size_t index, const Request& request, Response& res, bool& shouldAddContent);
	void HandleOptions(const Request& request, Response& res);
	void HandlePut(size_t index, const Request& request, Response& res);
	void HandleGet(size_t index, const Request& request, Response& res);
	void HandlePost(const Request& request, Response& res);
	void HandleDelete(size_t index, Response& res);
	void HandleTrace(const Request& request, Response& res);
	int ValidateRoute(const std::string& route, const std::string method); 
	std::string getUserData(SocketState& socketState); 
	bool isInactive(const SocketState& socket); 
};

#endif //SERVER_H