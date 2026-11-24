#!/usr/bin/env bash
set -e
echo "Compiling Student Attendance System (C++17 Backend)..."
g++ -std=c++17 -O2 -Wall -Wextra -IBackend \
    Backend/main.cpp \
    Backend/core/*.cpp \
    Backend/middlewares/*.cpp \
    Backend/utils/*.cpp \
    Backend/controllers/*.cpp \
    Backend/routes/*.cpp \
    -o attendance_server -lpthread

echo "Build succeeded! Launching server at http://localhost:8080 ..."
./attendance_server
