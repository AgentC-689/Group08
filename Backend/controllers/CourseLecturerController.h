#pragma once
#include "core/DatabaseContext.h"
#include "core/HttpTypes.h"
#include "middlewares/AuthMiddleware.h"

// Part 3 (Member 3): Business logic for Course Catalog & Lecturer Assigned Classes (FR06, FR08)
class CourseLecturerController {
private:
    DatabaseContext& db;

public:
    explicit CourseLecturerController(DatabaseContext& database);

    HttpResponse listCourses();
    HttpResponse createCourse(const HttpRequest& req);
    HttpResponse listLecturers();
    HttpResponse viewMyClasses(const AuthSession& session);
};
