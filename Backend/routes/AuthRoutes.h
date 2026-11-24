#pragma once
#include "controllers/AuthController.h"
#include "core/HttpServer.h"
#include "middlewares/AuthMiddleware.h"

// Part 1 (Member 1): Registers authentication and account management routes
void registerAuthRoutes(HttpServer& server, AuthController& controller, AuthMiddleware& auth);
