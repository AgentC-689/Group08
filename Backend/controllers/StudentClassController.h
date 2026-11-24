#pragma once
#include "core/DatabaseContext.h"
#include "core/HttpTypes.h"

// Part 2 (Member 2): Business logic for Student CRUD, CSV Import & Class Management (FR04, FR05, FR07)
class StudentClassController {
private:
    DatabaseContext& db;

public:
    explicit StudentClassController(DatabaseContext& database);

    HttpResponse listStudents(const HttpRequest& req);
    HttpResponse createStudent(const HttpRequest& req);
    HttpResponse deleteStudent(int studentId);
    HttpResponse importStudentsCsv(const HttpRequest& req);
    HttpResponse listClasses();
    HttpResponse createClass(const HttpRequest& req);
};
