#include "config/Config.h"
#include "controllers/AttendanceController.h"
#include "controllers/AuthController.h"
#include "controllers/CourseLecturerController.h"
#include "controllers/ReportController.h"
#include "controllers/StudentClassController.h"
#include "core/DatabaseContext.h"
#include "core/HttpServer.h"
#include "middlewares/AuthMiddleware.h"
#include "routes/AttendanceRoutes.h"
#include "routes/AuthRoutes.h"
#include "routes/CourseLecturerRoutes.h"
#include "routes/ReportRoutes.h"
#include "routes/StudentClassRoutes.h"

int main() {
    // Initialize Data Access Layer and RBAC Middleware
    DatabaseContext db;
    AuthMiddleware auth;

    // Initialize Business Logic Controllers (Parts 1 to 5)
    AuthController authController(db, auth);
    StudentClassController studentClassController(db);
    CourseLecturerController courseLecturerController(db);
    AttendanceController attendanceController(db);
    ReportController reportController(db);

    // Initialize HTTP Server and mount modular routes
    HttpServer server(Config::SERVER_PORT, Config::FRONTEND_DIR);
    registerAuthRoutes(server, authController, auth);
    registerStudentClassRoutes(server, studentClassController, auth);
    registerCourseLecturerRoutes(server, courseLecturerController, auth);
    registerAttendanceRoutes(server, attendanceController, auth);
    registerReportRoutes(server, reportController, auth);

    server.start();
    return 0;
}
