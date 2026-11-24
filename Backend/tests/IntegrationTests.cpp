#include "controllers/AttendanceController.h"
#include "controllers/AuthController.h"
#include "controllers/CourseLecturerController.h"
#include "controllers/ReportController.h"
#include "controllers/StudentClassController.h"
#include "core/DatabaseContext.h"
#include "core/HttpServer.h"
#include "core/JsonUtils.h"
#include "middlewares/AuthMiddleware.h"
#include "routes/AttendanceRoutes.h"
#include "routes/AuthRoutes.h"
#include "routes/CourseLecturerRoutes.h"
#include "routes/ReportRoutes.h"
#include "routes/StudentClassRoutes.h"
#include <cassert>
#include <iostream>

// Stage 4 Automated Integration Test Suite verifying FR01-FR16 & NFR03-NFR09
int main() {
    DatabaseContext db;
    AuthMiddleware auth;
    AuthController authCtrl(db, auth);
    StudentClassController scCtrl(db);
    CourseLecturerController clCtrl(db);
    AttendanceController attCtrl(db);
    ReportController repCtrl(db);

    HttpServer server(8080, "Frontend");
    registerAuthRoutes(server, authCtrl, auth);
    registerStudentClassRoutes(server, scCtrl, auth);
    registerCourseLecturerRoutes(server, clCtrl, auth);
    registerAttendanceRoutes(server, attCtrl, auth);
    registerReportRoutes(server, repCtrl, auth);

    auto makeReq = [](std::string method, std::string path, std::string body = "", std::string token = "") {
        HttpRequest req;
        req.method = std::move(method);
        req.path = std::move(path);
        req.body = std::move(body);
        if (!token.empty()) req.headers["authorization"] = "Bearer " + token;
        return req;
    };

    // TC01: Login for Admin, Lecturer, and Student (FR01)
    auto rAdmin = server.dispatch(makeReq("POST", "/api/auth/login", "{\"username\":\"admin\",\"password\":\"123456\"}"));
    assert(rAdmin.statusCode == 200);
    std::string adminToken = JsonUtils::getString(JsonUtils::parseFlatObject(rAdmin.body), "access_token");

    auto rLec = server.dispatch(makeReq("POST", "/api/auth/login", "{\"username\":\"gv001\",\"password\":\"123456\"}"));
    assert(rLec.statusCode == 200);
    std::string lecToken = JsonUtils::getString(JsonUtils::parseFlatObject(rLec.body), "access_token");

    auto rStu = server.dispatch(makeReq("POST", "/api/auth/login", "{\"username\":\"SV003\",\"password\":\"123456\"}"));
    assert(rStu.statusCode == 200);
    std::string stuToken = JsonUtils::getString(JsonUtils::parseFlatObject(rStu.body), "access_token");

    // TC02: RBAC enforcement (NFR04)
    assert(server.dispatch(makeReq("GET", "/api/accounts", "", stuToken)).statusCode == 403);

    // TC03: Password change validation (FR02)
    assert(server.dispatch(makeReq("POST", "/api/auth/change-password", "{\"old_password\":\"123456\",\"new_password\":\"newpass123\"}", stuToken)).statusCode == 200);

    // TC04: Schedule & room conflict rejection (FR07, UC04 2a)
    auto rConf = server.dispatch(makeReq("POST", "/api/classes",
        "{\"class_code\":\"CNTT99\",\"class_name\":\"Conflict\",\"course_id\":1,\"lecturer_id\":1,\"schedule\":\"Mon 07:00-09:30\",\"room\":\"A201\"}", adminToken));
    assert(rConf.statusCode == 409);

    // TC05: Session creation time validation & code generation (FR15, UC08)
    assert(server.dispatch(makeReq("POST", "/api/sessions",
        "{\"class_id\":1,\"session_date\":\"2026-11-23\",\"time_slot\":\"Slot 3\",\"start_time\":\"10:00\",\"end_time\":\"09:00\"}", lecToken)).statusCode == 400);

    auto rGoodSess = server.dispatch(makeReq("POST", "/api/sessions",
        "{\"class_id\":1,\"session_date\":\"2026-11-23\",\"time_slot\":\"Slot 3\",\"start_time\":\"00:00\",\"end_time\":\"23:59\",\"require_location\":true,\"class_latitude\":10.8037,\"class_longitude\":106.7168,\"allowed_distance\":100}", lecToken));
    assert(rGoodSess.statusCode == 201);
    auto sessObj = JsonUtils::parseFlatObject(rGoodSess.body);
    std::string sessId = JsonUtils::getString(sessObj, "session_id");
    std::string sessCode = JsonUtils::getString(sessObj, "session_code");

    // TC06: Student check-in outside 100m GPS radius -> flagged Outside class area (FR16, UC09 4d)
    auto rOut = server.dispatch(makeReq("POST", "/api/attendance/check-in",
        "{\"session_id\":" + sessId + ",\"session_code\":\"" + sessCode + "\",\"current_date\":\"2026-11-23\",\"current_time\":\"08:00\",\"device_latitude\":10.8200,\"device_longitude\":106.7500}", stuToken));
    assert(rOut.statusCode == 201);
    assert(rOut.body.find("\"is_outside_area\":true") != std::string::npos);

    // TC07: Close session -> auto-mark unchecked students Absent (FR16)
    auto rClose = server.dispatch(makeReq("POST", "/api/sessions/" + sessId + "/close", "", lecToken));
    assert(rClose.statusCode == 200);
    assert(rClose.body.find("\"auto_marked_absent\":2") != std::string::npos);

    // TC08: Edit attendance requires reason & stores audit log (FR10, NFR09)
    assert(server.dispatch(makeReq("PUT", "/api/attendance/records/2", "{\"status\":\"Late\"}", lecToken)).statusCode == 400);
    auto rEdit = server.dispatch(makeReq("PUT", "/api/attendance/records/2", "{\"status\":\"Late\",\"edit_reason\":\"Medical excuse verified\"}", lecToken));
    assert(rEdit.statusCode == 200);
    assert(rEdit.body.find("\"old_status\":\"Absent\"") != std::string::npos);

    // TC09: Attendance report & >20% absence warning (FR12, FR14)
    auto rRep = server.dispatch(makeReq("GET", "/api/reports", "", adminToken));
    assert(rRep.statusCode == 200);
    assert(rRep.body.find("\"absence_warning\":true") != std::string::npos);

    // TC10: Static Frontend serving check (index.html & change_password.html)
    auto rHtml = server.dispatch(makeReq("GET", "/"));
    assert(rHtml.statusCode == 200 && rHtml.body.find("Student Attendance System") != std::string::npos);
    auto rPwdHtml = server.dispatch(makeReq("GET", "/change_password.html"));
    assert(rPwdHtml.statusCode == 200 && rPwdHtml.body.find("Change Password") != std::string::npos);

    std::cout << "ALL 10 INTEGRATION TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}
