#pragma once
#include "controllers/StudentClassController.h"
#include "core/HttpServer.h"
#include "middlewares/AuthMiddleware.h"

// Part 2 (Member 2): Registers student roster and class management routes
void registerStudentClassRoutes(HttpServer& server, StudentClassController& controller, AuthMiddleware& auth);
