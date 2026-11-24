#pragma once
#include "core/DatabaseContext.h"
#include "core/HttpTypes.h"
#include "middlewares/AuthMiddleware.h"

// Part 5 (Member 5): Dashboard KPIs, Filtered History, Reports, >20% Warning & Export (FR11-FR14)
class ReportController {
private:
    DatabaseContext& db;

public:
    explicit ReportController(DatabaseContext& database);

    HttpResponse getDashboardSummary();
    HttpResponse getAttendanceHistory(const HttpRequest& req, const AuthSession& session);
    HttpResponse getAttendanceReport(const HttpRequest& req, const AuthSession& session);
    HttpResponse exportReportCsv(const HttpRequest& req, const AuthSession& session);
};
