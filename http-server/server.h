#ifndef SERVER_H
#define SERVER_H
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <regex>
#include <chrono>
#include <ctime>
#include <string>


#include "request.h"
#include "response.h"
#include "utils/StringUtils.h"

class HttpServer {
public:
	HttpServer(int port);
	~HttpServer();

	void Start();

private:
	const int MAX_SOCKETS = 60;
	enum SocketStates { EMPTY = 0, LISTEN = 1, RECEIVE = 2, HANDLE = 3, IDLE = 4, SEND = 5 };
	enum RoutesStatus { ROUTE_NOT_FOUND = -1, NOT_ALLOWED = 0, ALLOWED = 1 };
	enum HttpStatus {
		OK = 200,
		CREATED = 201,
		METHOD_NOT_ALLOWED = 405,
		NOT_FOUND = 404,
		NOT_IMPLEMENTED = 501
	};
	const std::string HTTP_OK = "200 OK";
	const std::string HTTP_CREATED = "200 CREATED";
	const std::string HTTP_NOT_FOUND = "404 NOT FOUND";
	const std::string HTTP_METHOD_NOT_ALLOWED = "405 METHOD NOT ALLOWED";
	const std::string HTTP_NOT_IMPLEMENTED = "501 NOT IMPLEMENTED";
	std::unordered_map<std::string, std::vector<std::string>> routes = {
		{"/",{"OPTIONS","GET","HEAD","TRACE"}},
		{"/cat-facts.html",{"OPTIONS","GET","HEAD","TRACE"}},
		{"/cat.jpg",{"OPTIONS","GET","HEAD","TRACE"}},
		{"/dog-facts.html",{"OPTIONS","GET","HEAD","TRACE"}},
		{"/dog.jpg",{"OPTIONS","GET","HEAD","TRACE"}},
		{"/my-name",{"OPTIONS","TRACE","GET","HEAD","PUT","DELETE"}},
		{"/post-comment",{"OPTIONS","POST"}}
	};

	std::unordered_map<std::string, std::string> mime_types = {
	{"txt", "text/plain"},
	{"html", "text/html"},
	{"jpg", "image/jpeg"},
	};

	int port_;
	SOCKET listenSocket_;

	struct SocketState {
		SOCKET id;
		int recv;
		int handle;
		int send;
		char buffer[1024];
		char res[1024];
		std::string name;
		int lenName;
		std::queue<Request> requests;
		int len;
		std::string ip;
		std::chrono::steady_clock::time_point lastActivityTime;
		// Function to update the last activity timestamp
		void updateLastActivity() {
			lastActivityTime = std::chrono::steady_clock::now();
		}
	};

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
	std::string HandleFileRead(std::ifstream& file);
	int ValidateRoute(const std::string& route, const std::string method); // TODO: move
	std::string getMimeTypeFromExtension(const std::string& filename); // TODO: move
	std::string getUserData(SocketState& socketState); // TODO: move
	bool isInactive(const SocketState& socket); // TODO: move
};

#endif //SERVER_H