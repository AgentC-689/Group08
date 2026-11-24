#pragma once
#include "controllers/CourseLecturerController.h"
#include "core/HttpServer.h"
#include "middlewares/AuthMiddleware.h"

// Part 3 (Member 3): Registers course catalog and lecturer assignment routes
void registerCourseLecturerRoutes(HttpServer& server, CourseLecturerController& controller, AuthMiddleware& auth);
