#pragma once
#include <map>
#include <string>

// Represents an incoming HTTP request parsed by HttpServer
struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> queryParams;
    std::map<std::string, std::string> headers;
    std::string body;
};

// Represents an outgoing HTTP response returned by route handlers
struct HttpResponse {
    int statusCode = 200;
    std::string contentType = "application/json; charset=utf-8";
    std::string body;
    std::map<std::string, std::string> extraHeaders;

    static HttpResponse json(int code, const std::string& jsonBody) {
        HttpResponse res;
        res.statusCode = code;
        res.contentType = "application/json; charset=utf-8";
        res.body = jsonBody;
        return res;
    }

    static HttpResponse error(int code, const std::string& message) {
        return json(code, "{\"error\":\"" + message + "\"}");
    }
};
