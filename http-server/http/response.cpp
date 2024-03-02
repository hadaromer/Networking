#include "response.h"
#include "../utils/Utils.h"

Response::Response() {}

std::string Response::getStatus() const {
	return this->status;
}

std::string Response::getContent() const {
	return this->content;
}

std::vector<Header> Response::getHeaders() const {
	return this->headers;
}


void Response::setContent(std::string content) {
	this->content = content;
}

void Response::setHeaders(std::vector<Header> headers) {
	this->headers = headers;
}

void Response::addHeader(const std::string& key, const std::string& value) {
	headers.push_back(Header(key, value));
}

void Response::setStatus(std::string status) {
	this->status = status;
}

void Response::setShouldIncludeContent(bool shouldIncludeContent) {
	this->shouldIncludeContent = shouldIncludeContent;
}

std::string Response::to_string() const {
	std::string response = "HTTP/1.1 " + this->getStatus() + "\n";
	for (Header header : this->getHeaders()) response += header.to_string() + "\n";
	response += "Server: MTA-HTTP-SERVER\n";
	response += "Date: " + Utils().getCurrentDate();
	response += "Content-Length: " + std::to_string(this->content.length()) + "\n";
	if (this->shouldIncludeContent) {
		return response + "\n" + this->content;
	}
	return response + "\n";
}

