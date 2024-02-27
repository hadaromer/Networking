#include "request.h"

Request::Request(char buffer[1024]) {
    originalRequest = std::string(buffer);
    std::vector<std::string> lines = StringUtils().splitString(buffer,'\n');
    std::vector<std::string> head = StringUtils().split(lines[0]);

    this->method = head[0];
    this->url = head[1];
    this->version = head[2];
    
    // Extract path and query parameters
    int questionMarkPos = this->url.find('?');
    this->path = this->url.substr(0, questionMarkPos);
    std::string queryString = (questionMarkPos != std::string::npos) ? url.substr(questionMarkPos + 1) : "";
    this->queryParams = parseQueryParams(queryString);

    int content_index = 0;

    for (int i = 1; i < lines.size(); i++) {
        if (lines[i][0] == '\r') { 
            content_index = i + 1;
            break;
        }
        std::vector<std::string> header = StringUtils().split(lines[i]);
        std::string key = header[0];
        key.pop_back();

        std::string value;
        header.erase(header.begin());
        for (const auto& s : header) value += s + " ";
        value.pop_back();

        this->headers.push_back(Header(key, value));
    }

    for (int i = content_index; i < lines.size(); i++) content.push_back(lines[i]);
}

std::string Request::getMethod(void) const {
    return this->method;
}

std::string Request::getPath(void) const {
    return this->path;
}

std::string Request::getVersion(void) const {
    return this->version;
}

std::vector<Header> Request::getHeaders(void) const {
    return this->headers;
}

std::vector<std::string> Request::getContent(void) const {
    return this->content;
}

Header Request::getHeader(std::string key) const {
    for (Header header : this->getHeaders()) {
        if (header.get_key() == key) return header;
    }

    return Header("", "");
}

std::string Request::getOriginalRequest() const {
    return originalRequest;
}

std::unordered_map<std::string, std::string> Request::parseQueryParams(const std::string& queryString) {
    std::unordered_map<std::string, std::string> queryParams;

    std::istringstream ss(queryString);
    std::string token;

    while (std::getline(ss, token, '&')) {
        size_t equalPos = token.find('=');
        if (equalPos != std::string::npos) {
            std::string key = token.substr(0, equalPos);
            std::string value = token.substr(equalPos + 1);
            queryParams[key] = value;
        }
    }

    return queryParams;
}

std::string Request::getQueryParam(std::string key) const {
    auto selectedQueryParam = this->queryParams.find(key);
    if (selectedQueryParam == this->queryParams.end()) {
        return "";
    }
    else {
        return selectedQueryParam->second;
    }
}