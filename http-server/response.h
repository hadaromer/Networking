#ifndef RESPONSE_H
#define RESPONSE_H

#include <vector>
#include <string>
#include "header.h"

    class Response {

    public:
        Response();

        std::string getStatus() const;
        std::string getContent() const;
        std::vector<Header> getHeaders() const;

        void setStatus(std::string status);
        void setContent(std::string content);
        void setHeaders(std::vector<Header> headers);
        void addHeader(const std::string& key, const std::string& value);
        std::string to_string(bool shouldAddContent) const;

    private:
        std::string status;
        std::string content;
        std::vector<Header> headers = {};
    };


#endif //RESPONSE_H