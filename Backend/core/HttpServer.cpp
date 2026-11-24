#include "core/HttpServer.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using SocketType = SOCKET;
    static void closeSocket(SocketType s) { closesocket(s); }
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    using SocketType = int;
    static constexpr SocketType INVALID_SOCKET = -1;
    static void closeSocket(SocketType s) { close(s); }
#endif

static std::vector<std::string> splitBy(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) parts.push_back(item);
    }
    return parts;
}

static std::string urlDecode(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int val = std::stoi(s.substr(i + 1, 2), nullptr, 16);
            out += static_cast<char>(val);
            i += 2;
        } else if (s[i] == '+') {
            out += ' ';
        } else {
            out += s[i];
        }
    }
    return out;
}

HttpServer::HttpServer(int listenPort, std::string frontendDir)
    : port(listenPort), staticDir(std::move(frontendDir)) {}

void HttpServer::addRoute(const std::string& method, const std::string& pattern, RouteHandler handler) {
    routes.push_back({method, pattern, std::move(handler)});
}

bool HttpServer::matchPattern(const std::string& pattern, const std::string& path, std::vector<std::string>& outParams) {
    outParams.clear();
    auto pSegs = splitBy(pattern, '/');
    auto rSegs = splitBy(path, '/');
    if (pSegs.size() != rSegs.size()) return false;
    for (size_t i = 0; i < pSegs.size(); ++i) {
        if (!pSegs[i].empty() && pSegs[i][0] == ':') {
            outParams.push_back(rSegs[i]);
        } else if (pSegs[i] != rSegs[i]) {
            return false;
        }
    }
    return true;
}

HttpRequest HttpServer::parseRawRequest(const std::string& raw) {
    HttpRequest req;
    size_t headerEnd = raw.find("\r\n\r\n");
    std::string headerPart = (headerEnd == std::string::npos) ? raw : raw.substr(0, headerEnd);
    if (headerEnd != std::string::npos) req.body = raw.substr(headerEnd + 4);

    std::istringstream iss(headerPart);
    std::string requestLine;
    std::getline(iss, requestLine);
    if (!requestLine.empty() && requestLine.back() == '\r') requestLine.pop_back();

    std::istringstream rls(requestLine);
    std::string fullPath;
    rls >> req.method >> fullPath;

    size_t qPos = fullPath.find('?');
    if (qPos != std::string::npos) {
        req.path = fullPath.substr(0, qPos);
        std::string query = fullPath.substr(qPos + 1);
        for (const auto& pair : splitBy(query, '&')) {
            size_t eq = pair.find('=');
            if (eq != std::string::npos) {
                req.queryParams[urlDecode(pair.substr(0, eq))] = urlDecode(pair.substr(eq + 1));
            }
        }
    } else {
        req.path = fullPath;
    }

    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            while (!val.empty() && val.front() == ' ') val.erase(val.begin());
            req.headers[key] = val;
        }
    }
    return req;
}

HttpResponse HttpServer::serveStaticFile(const std::string& path) const {
    std::string rel = (path == "/" || path.empty()) ? "/index.html" : path;
    if (rel.find("..") != std::string::npos) return HttpResponse::error(403, "Forbidden path");

    std::ifstream file(staticDir + rel, std::ios::binary);
    if (!file.is_open()) {
        // Fallback when executable is launched from build/ or Backend/ subdirectory
        file.open("../" + staticDir + rel, std::ios::binary);
    }
    if (!file.is_open()) return HttpResponse::error(404, "Resource not found");

    std::ostringstream buffer;
    buffer << file.rdbuf();

    HttpResponse res;
    res.statusCode = 200;
    res.body = buffer.str();
    if (rel.size() >= 5 && rel.substr(rel.size() - 5) == ".html") res.contentType = "text/html; charset=utf-8";
    else if (rel.size() >= 4 && rel.substr(rel.size() - 4) == ".css") res.contentType = "text/css; charset=utf-8";
    else if (rel.size() >= 3 && rel.substr(rel.size() - 3) == ".js") res.contentType = "application/javascript; charset=utf-8";
    else res.contentType = "application/octet-stream";
    return res;
}

HttpResponse HttpServer::dispatch(const HttpRequest& req) const {
    if (req.method == "OPTIONS") {
        HttpResponse res;
        res.statusCode = 204;
        return res;
    }
    for (const auto& entry : routes) {
        std::vector<std::string> params;
        if (entry.method == req.method && matchPattern(entry.pattern, req.path, params)) {
            return entry.handler(req, params);
        }
    }
    if (req.method == "GET" && req.path.rfind("/api/", 0) != 0) {
        return serveStaticFile(req.path);
    }
    return HttpResponse::error(404, "Endpoint not found");
}

std::string HttpServer::serializeResponse(const HttpResponse& res) {
    std::string reason = "OK";
    if (res.statusCode == 201) reason = "Created";
    else if (res.statusCode == 204) reason = "No Content";
    else if (res.statusCode == 400) reason = "Bad Request";
    else if (res.statusCode == 401) reason = "Unauthorized";
    else if (res.statusCode == 403) reason = "Forbidden";
    else if (res.statusCode == 404) reason = "Not Found";
    else if (res.statusCode == 409) reason = "Conflict";

    std::ostringstream oss;
    oss << "HTTP/1.1 " << res.statusCode << " " << reason << "\r\n"
        << "Content-Type: " << res.contentType << "\r\n"
        << "Content-Length: " << res.body.size() << "\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Access-Control-Allow-Methods: GET, POST, PUT, PATCH, DELETE, OPTIONS\r\n"
        << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        << "Connection: close\r\n";
    for (const auto& kv : res.extraHeaders) {
        oss << kv.first << ": " << kv.second << "\r\n";
    }
    oss << "\r\n" << res.body;
    return oss.str();
}

void HttpServer::start() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    SocketType serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == INVALID_SOCKET) {
        std::cerr << "Failed to create server socket.\n";
        return;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(serverFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "Failed to bind port " << port << ".\n";
        closeSocket(serverFd);
        return;
    }

    listen(serverFd, 64);
    std::cout << "Student Attendance System C++ Server running at http://localhost:" << port << "\n";

    while (true) {
        SocketType clientFd = accept(serverFd, nullptr, nullptr);
        if (clientFd == INVALID_SOCKET) continue;

        std::thread([this, clientFd]() {
            std::string raw;
            char buffer[4096];
            int bytes = recv(clientFd, buffer, sizeof(buffer), 0);
            if (bytes > 0) {
                raw.append(buffer, bytes);
                size_t headerEnd = raw.find("\r\n\r\n");
                if (headerEnd != std::string::npos) {
                    HttpRequest tmp = parseRawRequest(raw);
                    auto clIt = tmp.headers.find("content-length");
                    if (clIt != tmp.headers.end()) {
                        size_t expectedBody = std::stoul(clIt->second);
                        while (raw.size() < headerEnd + 4 + expectedBody) {
                            int more = recv(clientFd, buffer, sizeof(buffer), 0);
                            if (more <= 0) break;
                            raw.append(buffer, more);
                        }
                    }
                }
                HttpRequest req = parseRawRequest(raw);
                HttpResponse res = dispatch(req);
                std::string payload = serializeResponse(res);
                send(clientFd, payload.c_str(), static_cast<int>(payload.size()), 0);
            }
            closeSocket(clientFd);
        }).detach();
    }
}
