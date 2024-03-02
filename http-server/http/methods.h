#ifndef METHODS_H
#define METHODS_H

#include <regex>

#include "../common/consts.h"
#include "../utils/Utils.h"

void HandleOptions(SocketState& socket, const Request& req, Response& res);
void HandlePut(SocketState& socket, const Request& req, Response& res);
void HandleGetName(SocketState& socket, const Request& req, Response& res);
void HandleGet(SocketState& socket, const Request& req, Response& res);
void HandlePost(SocketState& socket, const Request& req, Response& res);
void HandleDelete(SocketState& socket, const Request& req, Response& res);
void HandleTrace(SocketState& socket, const Request& req, Response& res);


const std::unordered_map<std::string, std::vector<Method>> routes = {
	{"/",{{"OPTIONS",HandleOptions},{"GET",HandleGet},{"HEAD",HandleGet},{"TRACE",HandleTrace}} },
	{"/cat-facts.html",{{"OPTIONS",HandleOptions},{"GET",HandleGet},{"HEAD",HandleGet},{"TRACE",HandleTrace}}},
	{"/cat.jpg",{{"OPTIONS",HandleOptions},{"GET",HandleGet},{"HEAD",HandleGet},{"TRACE",HandleTrace}}},
	{"/dog-facts.html",{{"OPTIONS",HandleOptions},{"GET",HandleGet},{"HEAD",HandleGet},{"TRACE",HandleTrace}}},
	{"/dog.jpg",{{"OPTIONS",HandleOptions},{"GET",HandleGet},{"HEAD",HandleGet},{"TRACE",HandleTrace}}},
	{"/my-name",{{"OPTIONS",HandleOptions},{"TRACE",HandleTrace},{"GET",HandleGetName},{"HEAD",HandleGetName},{"PUT",HandlePut},{"DELETE",HandleDelete}}},
	{"/post-comment",{{"OPTIONS",HandleOptions} ,{"TRACE",HandleTrace},{"POST",HandlePost}}}
};


#endif //METHODS_H