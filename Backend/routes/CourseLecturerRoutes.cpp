#include "routes/CourseLecturerRoutes.h"

void registerCourseLecturerRoutes(HttpServer& server, CourseLecturerController& controller, AuthMiddleware& auth) {
    server.addRoute("GET", "/api/courses", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator", "Lecturer"}, sess, err)) return err;
        return controller.listCourses();
    });

    server.addRoute("POST", "/api/courses", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.createCourse(req);
    });

    server.addRoute("GET", "/api/lecturers", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Administrator"}, sess, err)) return err;
        return controller.listLecturers();
    });

    server.addRoute("GET", "/api/lecturers/my-classes", [&](const HttpRequest& req, const std::vector<std::string>&) {
        AuthSession sess; HttpResponse err;
        if (!auth.authorize(req, {"Lecturer"}, sess, err)) return err;
        return controller.viewMyClasses(sess);
    });
}
