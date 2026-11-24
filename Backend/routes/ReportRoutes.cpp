#include "routes/ReportRoutes.h"

void registerReportRoutes(HttpServer& server, ReportController& controller, AuthMiddleware& auth) {
    server.addRoute("GET", "/api/dashboard/summary", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator", "Lecturer", "Student"}, sess, err)) return err;
        return controller.getDashboardSummary();
    });

    server.addRoute("GET", "/api/history", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator", "Lecturer", "Student"}, sess, err)) return err;
        return controller.getAttendanceHistory(req, sess);
    });

    server.addRoute("GET", "/api/reports", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator", "Lecturer", "Student"}, sess, err)) return err;
        return controller.getAttendanceReport(req, sess);
    });

    server.addRoute("GET", "/api/reports/export", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator", "Lecturer"}, sess, err)) return err;
        return controller.exportReportCsv(req, sess);
    });
}
