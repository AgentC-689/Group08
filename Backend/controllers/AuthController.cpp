#include "controllers/AuthController.h"
#include "core/JsonUtils.h"
#include "core/SecurityUtils.h"
#include <sstream>

AuthController::AuthController(DatabaseContext& database, AuthMiddleware& authMiddleware)
    : db(database), auth(authMiddleware) {}

HttpResponse AuthController::login(const HttpRequest& req) {
    auto body = JsonUtils::parseFlatObject(req.body);
    std::string username = JsonUtils::getString(body, "username");
    std::string password = JsonUtils::getString(body, "password");

    std::lock_guard<std::mutex> lock(db.mtx);
    User* user = db.findUserByUsername(username);
    // UC01 Alternative Flow 3a: Wrong credentials
    if (!user || !SecurityUtils::verifyPassword(password, user->passwordHash)) {
        return HttpResponse::error(401, "Wrong username or password");
    }
    // UC01 Alternative Flow 3b: Locked account
    if (user->isLocked) {
        return HttpResponse::error(403, "Account is locked. Contact Administrator.");
    }

    std::string token = auth.createSession(user->userId, user->username, user->role);
    std::ostringstream oss;
    oss << "{\"access_token\":" << JsonUtils::quote(token)
        << ",\"user_id\":" << user->userId
        << ",\"username\":" << JsonUtils::quote(user->username)
        << ",\"role\":" << JsonUtils::quote(user->role) << "}";
    return HttpResponse::json(200, oss.str());
}

HttpResponse AuthController::logout(const HttpRequest& req) {
    auto it = req.headers.find("authorization");
    if (it != req.headers.end() && it->second.rfind("Bearer ", 0) == 0) {
        auth.invalidateSession(it->second.substr(7));
    }
    return HttpResponse::json(200, "{\"message\":\"Logged out successfully\"}");
}

HttpResponse AuthController::changePassword(const HttpRequest& req, const AuthSession& session) {
    auto body = JsonUtils::parseFlatObject(req.body);
    std::string oldPass = JsonUtils::getString(body, "old_password");
    std::string newPass = JsonUtils::getString(body, "new_password");
    if (newPass.size() < 6) {
        return HttpResponse::error(400, "New password must be at least 6 characters");
    }

    std::lock_guard<std::mutex> lock(db.mtx);
    User* user = db.findUserById(session.userId);
    if (!user || !SecurityUtils::verifyPassword(oldPass, user->passwordHash)) {
        return HttpResponse::error(400, "Current password is incorrect");
    }
    user->passwordHash = SecurityUtils::hashPassword(newPass);
    return HttpResponse::json(200, "{\"message\":\"Password updated successfully\"}");
}

HttpResponse AuthController::listAccounts() {
    std::lock_guard<std::mutex> lock(db.mtx);
    std::vector<std::string> items;
    for (const auto& u : db.users) {
        std::ostringstream oss;
        oss << "{\"user_id\":" << u.userId
            << ",\"username\":" << JsonUtils::quote(u.username)
            << ",\"role\":" << JsonUtils::quote(u.role)
            << ",\"is_locked\":" << (u.isLocked ? "true" : "false")
            << ",\"created_at\":" << JsonUtils::quote(u.createdAt) << "}";
        items.push_back(oss.str());
    }
    return HttpResponse::json(200, JsonUtils::joinArray(items));
}

HttpResponse AuthController::createAccount(const HttpRequest& req) {
    auto body = JsonUtils::parseFlatObject(req.body);
    std::string username = JsonUtils::getString(body, "username");
    std::string password = JsonUtils::getString(body, "password", "123456");
    std::string role = JsonUtils::getString(body, "role");

    if (username.empty() || (role != "Administrator" && role != "Lecturer" && role != "Student")) {
        return HttpResponse::error(400, "Valid username and role (Administrator, Lecturer, Student) are required");
    }

    std::lock_guard<std::mutex> lock(db.mtx);
    // UC02 Alternative Flow 4a: Duplicate username check
    if (db.findUserByUsername(username)) {
        return HttpResponse::error(409, "Username already exists");
    }
    int uid = db.nextUserId++;
    db.users.emplace_back(uid, username, SecurityUtils::hashPassword(password), role, false, SecurityUtils::getCurrentDateTimeString());
    return HttpResponse::json(201, "{\"message\":\"Account created\",\"user_id\":" + std::to_string(uid) + "}");
}

HttpResponse AuthController::toggleAccountLock(int userId) {
    std::lock_guard<std::mutex> lock(db.mtx);
    User* user = db.findUserById(userId);
    if (!user) return HttpResponse::error(404, "Account not found");
    user->isLocked = !user->isLocked;
    return HttpResponse::json(200, "{\"user_id\":" + std::to_string(userId) + ",\"is_locked\":" + (user->isLocked ? "true" : "false") + "}");
}

HttpResponse AuthController::resetPassword(int userId, const HttpRequest& req) {
    auto body = JsonUtils::parseFlatObject(req.body);
    std::string newPass = JsonUtils::getString(body, "new_password", "123456");
    std::lock_guard<std::mutex> lock(db.mtx);
    User* user = db.findUserById(userId);
    if (!user) return HttpResponse::error(404, "Account not found");
    user->passwordHash = SecurityUtils::hashPassword(newPass);
    return HttpResponse::json(200, "{\"message\":\"Password reset successfully\"}");
}
