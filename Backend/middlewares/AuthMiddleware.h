#pragma once
#include "core/HttpTypes.h"
#include <ctime>
#include <map>
#include <mutex>
#include <string>
#include <vector>

struct AuthSession {
    std::string token;
    int userId = 0;
    std::string username;
    std::string role;
    std::time_t lastActivity = 0;
};

// Handles Bearer token validation, 30-min inactivity expiration (NFR03), and RBAC (NFR04)
class AuthMiddleware {
private:
    std::mutex sessionMtx;
    std::map<std::string, AuthSession> activeSessions;

public:
    std::string createSession(int userId, const std::string& username, const std::string& role);
    void invalidateSession(const std::string& token);
    bool authenticate(const HttpRequest& req, AuthSession& outSession, HttpResponse& errRes);
    bool authorize(const HttpRequest& req, const std::vector<std::string>& allowedRoles,
                   AuthSession& outSession, HttpResponse& errRes);
};
