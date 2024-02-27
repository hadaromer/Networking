#ifndef REQUEST_H
#define REQUEST_H

#include "utils/StringUtils.h"
#include "header.h"
#include <unordered_map>

class Request {

public:
	Request(char buffer[1024]);
	std::string getMethod(void) const;
	std::string getPath(void) const;
	std::string getVersion(void) const;
	std::vector<Header> getHeaders(void) const;
	std::vector<std::string> getContent(void) const;
	Header getHeader(std::string key) const;
	std::string getOriginalRequest() const;
	std::string getQueryParam(std::string key) const;

private:
	std::string originalRequest;
	std::string method;
	std::string url;
	std::string path;
	std::unordered_map<std::string, std::string> queryParams;
	std::string version;
	std::vector<Header> headers = {};
	std::vector<std::string> content = {};
	std::unordered_map<std::string, std::string> parseQueryParams(const std::string& queryString);
};

#endif //REQUEST_H