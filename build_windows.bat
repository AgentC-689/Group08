@echo off
echo Compiling Student Attendance System (C++17 Backend)...
g++ -std=c++17 -O2 -Wall -Wextra -IBackend ^
    Backend/main.cpp ^
    Backend/core/*.cpp ^
    Backend/middlewares/*.cpp ^
    Backend/utils/*.cpp ^
    Backend/controllers/*.cpp ^
    Backend/routes/*.cpp ^
    -o attendance_server.exe -lws2_32

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed! Ensure MinGW-w64 g++ is installed and on PATH.
    pause
    exit /b %ERRORLEVEL%
)

echo Build succeeded! Launching server at http://localhost:8080 ...
attendance_server.exe
