#pragma once
#include "core/HttpTypes.h"
#include <functional>
#include <string>
#include <vector>

using RouteHandler = std::function<HttpResponse(const HttpRequest&, const std::vector<std::string>&)>;

struct RouteEntry {
    std::string method;
    std::string pattern; // Supports path parameters like /api/accounts/:id/lock
    RouteHandler handler;
};

// Cross-platform HTTP/1.1 server supporting REST APIs and static Web Frontend files
class HttpServer {
private:
    int port;
    std::string staticDir;
    std::vector<RouteEntry> routes;

    static bool matchPattern(const std::string& pattern, const std::string& path, std::vector<std::string>& outParams);
    static HttpRequest parseRawRequest(const std::string& raw);
    static std::string serializeResponse(const HttpResponse& res);
    HttpResponse serveStaticFile(const std::string& path) const;

public:
    HttpServer(int listenPort, std::string frontendDir);
    void addRoute(const std::string& method, const std::string& pattern, RouteHandler handler);
    HttpResponse dispatch(const HttpRequest& req) const;
    void start();
};
