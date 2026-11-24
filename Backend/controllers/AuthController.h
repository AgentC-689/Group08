#pragma once
#include "core/DatabaseContext.h"
#include "core/HttpTypes.h"
#include "middlewares/AuthMiddleware.h"

// Part 1 (Member 1): Business logic for Login, Logout, Password Change, and Account CRUD (FR01-FR03)
class AuthController {
private:
    DatabaseContext& db;
    AuthMiddleware& auth;

public:
    AuthController(DatabaseContext& database, AuthMiddleware& authMiddleware);

    HttpResponse login(const HttpRequest& req);
    HttpResponse logout(const HttpRequest& req);
    HttpResponse changePassword(const HttpRequest& req, const AuthSession& session);
    HttpResponse listAccounts();
    HttpResponse createAccount(const HttpRequest& req);
    HttpResponse toggleAccountLock(int userId);
    HttpResponse resetPassword(int userId, const HttpRequest& req);
};
