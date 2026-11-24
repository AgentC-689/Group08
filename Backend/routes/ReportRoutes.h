#pragma once
#include "controllers/ReportController.h"
#include "core/HttpServer.h"
#include "middlewares/AuthMiddleware.h"

// Part 5 (Member 5): Registers dashboard KPI, history, report, and export routes
void registerReportRoutes(HttpServer& server, ReportController& controller, AuthMiddleware& auth);
