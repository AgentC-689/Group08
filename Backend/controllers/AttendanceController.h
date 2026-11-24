#pragma once
#include "core/DatabaseContext.h"
#include "core/HttpTypes.h"
#include "middlewares/AuthMiddleware.h"

// Part 4 (Member 4): Core Attendance Engine (FR09, FR10, FR15, FR16, UC05, UC08, UC09, NFR09)
class AttendanceController {
private:
    DatabaseContext& db;

public:
    explicit AttendanceController(DatabaseContext& database);

    HttpResponse listSessions(const AuthSession& session);
    HttpResponse createSession(const HttpRequest& req, const AuthSession& session);
    HttpResponse closeSessionAndMarkAbsent(int sessionId, const AuthSession& session);
    HttpResponse getSessionRoster(int sessionId);
    HttpResponse takeAttendance(int sessionId, const HttpRequest& req, const AuthSession& session);
    HttpResponse studentCheckIn(const HttpRequest& req, const AuthSession& session);
    HttpResponse editAttendanceRecord(int recordId, const HttpRequest& req, const AuthSession& session);
};
