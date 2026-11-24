#include "routes/AuthRoutes.h"

void registerAuthRoutes(HttpServer& server, AuthController& controller, AuthMiddleware& auth) {
    server.addRoute("POST", "/api/auth/login", [&](const HttpRequest& req, const std::vector<std::string>&) {
        return controller.login(req);
    });

    server.addRoute("POST", "/api/auth/logout", [&](const HttpRequest& req, const std::vector<std::string>&) {
        return controller.logout(req);
    });

    server.addRoute("POST", "/api/auth/change-password", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authenticate(req, sess, err)) return err;
        return controller.changePassword(req, sess);
    });

    server.addRoute("GET", "/api/accounts", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.listAccounts();
    });

    server.addRoute("POST", "/api/accounts", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.createAccount(req);
    });

    server.addRoute("PATCH", "/api/accounts/:id/lock", [&](const HttpRequest& req, const std::vector<std::string>& params) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.toggleAccountLock(std::stoi(params[0]));
    });

    server.addRoute("POST", "/api/accounts/:id/reset-password", [&](const HttpRequest& req, const std::vector<std::string>& params) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.resetPassword(std::stoi(params[0]), req);
    });
}
