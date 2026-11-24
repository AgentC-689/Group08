#include "middlewares/AuthMiddleware.h"
#include "config/Config.h"
#include "core/SecurityUtils.h"
#include <algorithm>

std::string AuthMiddleware::createSession(int userId, const std::string& username, const std::string& role) {
    std::lock_guard<std::mutex> lock(sessionMtx);
    AuthSession sess;
    sess.token = SecurityUtils::generateRandomToken(40);
    sess.userId = userId;
    sess.username = username;
    sess.role = role;
    sess.lastActivity = std::time(nullptr);
    activeSessions[sess.token] = sess;
    return sess.token;
}

void AuthMiddleware::invalidateSession(const std::string& token) {
    std::lock_guard<std::mutex> lock(sessionMtx);
    activeSessions.erase(token);
}

bool AuthMiddleware::authenticate(const HttpRequest& req, AuthSession& outSession, HttpResponse& errRes) {
    auto it = req.headers.find("authorization");
    if (it == req.headers.end() || it->second.rfind("Bearer ", 0) != 0) {
        errRes = HttpResponse::error(401, "Missing or invalid Authorization header");
        return false;
    }

    const std::string token = it->second.substr(7);
    std::lock_guard<std::mutex> lock(sessionMtx);
    auto sessIt = activeSessions.find(token);
    if (sessIt == activeSessions.end()) {
        errRes = HttpResponse::error(401, "Invalid or expired session token");
        return false;
    }

    const std::time_t now = std::time(nullptr);
    if (now - sessIt->second.lastActivity > Config::SESSION_TIMEOUT_SECONDS) {
        activeSessions.erase(sessIt);
        errRes = HttpResponse::error(401, "Session expired after 30 minutes of inactivity");
        return false;
    }

    sessIt->second.lastActivity = now;
    outSession = sessIt->second;
    return true;
}

bool AuthMiddleware::authorize(const HttpRequest& req, const std::vector<std::string>& allowedRoles,
                               AuthSession& outSession, HttpResponse& errRes) {
    if (!authenticate(req, outSession, errRes)) return false;
    if (std::find(allowedRoles.begin(), allowedRoles.end(), outSession.role) == allowedRoles.end()) {
        errRes = HttpResponse::error(403, "Access denied: insufficient role permissions");
        return false;
    }
    return true;
}
