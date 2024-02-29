#include "response.h"


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

std::string Response::to_string(bool shouldAddContent) const {
	std::string response = "HTTP/1.1 " + this->getStatus() + "\n";
	for (Header header : this->getHeaders()) response += header.to_string() + "\n";
	response += "Server: MTA-HTTP-SERVER\n";
	response += "Date: " + Utils().getCurrentDate();
	response += "Content-Length: " + std::to_string(this->content.length()) + "\n";
	if (shouldAddContent) {
		return response + "\n" + this->content;
	}
	return response + "\n";
}

