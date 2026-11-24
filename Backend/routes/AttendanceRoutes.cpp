#include "routes/AttendanceRoutes.h"

void registerAttendanceRoutes(HttpServer& server, AttendanceController& controller, AuthMiddleware& auth) {
    server.addRoute("GET", "/api/sessions", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator", "Lecturer", "Student"}, sess, err)) return err;
        return controller.listSessions(sess);
    });

    server.addRoute("POST", "/api/sessions", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Lecturer"}, sess, err)) return err;
        return controller.createSession(req, sess);
    });

    server.addRoute("POST", "/api/sessions/:id/close", [&](const HttpRequest& req, const std::vector<std::string>& params) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Lecturer"}, sess, err)) return err;
        return controller.closeSessionAndMarkAbsent(std::stoi(params[0]), sess);
    });

    server.addRoute("GET", "/api/sessions/:id/roster", [&](const HttpRequest& req, const std::vector<std::string>& params) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator", "Lecturer"}, sess, err)) return err;
        return controller.getSessionRoster(std::stoi(params[0]));
    });

    server.addRoute("POST", "/api/sessions/:id/attendance", [&](const HttpRequest& req, const std::vector<std::string>& params) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Lecturer"}, sess, err)) return err;
        return controller.takeAttendance(std::stoi(params[0]), req, sess);
    });

    server.addRoute("POST", "/api/attendance/check-in", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Student"}, sess, err)) return err;
        return controller.studentCheckIn(req, sess);
    });

    server.addRoute("PUT", "/api/attendance/records/:id", [&](const HttpRequest& req, const std::vector<std::string>& params) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Lecturer"}, sess, err)) return err;
        return controller.editAttendanceRecord(std::stoi(params[0]), req, sess);
    });
}
