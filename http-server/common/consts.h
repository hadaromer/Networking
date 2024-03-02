#ifndef CONSTS_H
#define CONSTS_H

#include "string"
#include <queue>
#include <unordered_map>
#include "../http/request.h"
#include "../http/response.h"
#include <chrono>
#include <ctime>
#include <winsock2.h>

const int MAX_SOCKETS = 60;
enum SocketStates { EMPTY = 0, LISTEN = 1, RECEIVE = 2, SEND = 3, IDLE = 4 };
enum RoutesStatus { ROUTE_NOT_FOUND = -1, NOT_ALLOWED = 0, ALLOWED = 1 };

const std::string HTTP_OK = "200 OK";
const std::string HTTP_CREATED = "201 CREATED";
const std::string HTTP_BAD_REQUEST = "400 BAD REQUEST";
const std::string HTTP_NOT_FOUND = "404 NOT FOUND";
const std::string HTTP_METHOD_NOT_ALLOWED = "405 METHOD NOT ALLOWED";


struct SocketState {
	SOCKET id;
	int recv;
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

typedef void (*HandleFunction)(SocketState& socket, const Request& req, Response& res);

struct Method {
	std::string methodName;
	HandleFunction handleFunction;
};

const std::string RESOURCES_PATH = "html_pages\\";

const struct timeval SELECT_TIMEOUT { 60, 0 }; // one minute timeout for select

#endif //CONSTS_H