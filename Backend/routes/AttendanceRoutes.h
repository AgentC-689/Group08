#pragma once
#include "controllers/AttendanceController.h"
#include "core/HttpServer.h"
#include "middlewares/AuthMiddleware.h"

// Part 4 (Member 4): Registers attendance session, GPS check-in, and edit audit routes
void registerAttendanceRoutes(HttpServer& server, AttendanceController& controller, AuthMiddleware& auth);
