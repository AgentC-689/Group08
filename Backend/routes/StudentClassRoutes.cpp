#include "routes/StudentClassRoutes.h"

void registerStudentClassRoutes(HttpServer& server, StudentClassController& controller, AuthMiddleware& auth) {
    server.addRoute("GET", "/api/students", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator", "Lecturer"}, sess, err)) return err;
        return controller.listStudents(req);
    });

    server.addRoute("POST", "/api/students", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.createStudent(req);
    });

    server.addRoute("DELETE", "/api/students/:id", [&](const HttpRequest& req, const std::vector<std::string>& params) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.deleteStudent(std::stoi(params[0]));
    });

    server.addRoute("POST", "/api/students/import", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.importStudentsCsv(req);
    });

    server.addRoute("GET", "/api/classes", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator", "Lecturer", "Student"}, sess, err)) return err;
        return controller.listClasses();
    });

    server.addRoute("POST", "/api/classes", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.createClass(req);
    });
}
