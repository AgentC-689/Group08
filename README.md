# Student Attendance System

Complete full-stack project organized into three top-level directories: **`Frontend/`**, **`Backend/`** (C++17 layered MVC), and **`Database/`** (3NF SQL schemas and seed data). All source code, comments, SQL scripts, and UI screens are written in English and aligned with the project SRS (`FR01-FR16`, `NFR01-NFR10`, `UC01-UC09`).

## Directory Structure

```text
Student_Attendance_System/
├── Frontend/              # Responsive HTML5, CSS3 & modular JavaScript views (Figma-aligned)
│   ├── css/styles.css
│   ├── js/                # apiClient.js, layout.js, auth.js, studentClass.js, courseLecturer.js, attendance.js, reports.js
│   └── *.html             # Login, Dashboard, Accounts, Students, Classes, Courses, Attendance, Check-In, History, Reports, Change Password
├── Backend/               # C++17 Layered MVC REST API & Static File Server
│   ├── main.cpp
│   ├── config/            # Config.h
│   ├── core/              # HttpTypes, HttpServer, JsonUtils, SecurityUtils, DatabaseContext
│   ├── middlewares/       # AuthMiddleware (Bearer token, 30-min timeout, RBAC)
│   ├── models/            # User, Administrator, Lecturer, Student, Course, ClassModel, AttendanceSession, AttendanceRecord
│   ├── utils/             # GeoUtils (Haversine GPS distance calculation)
│   ├── controllers/       # AuthController, StudentClassController, CourseLecturerController, AttendanceController, ReportController
│   ├── routes/            # AuthRoutes, StudentClassRoutes, CourseLecturerRoutes, AttendanceRoutes, ReportRoutes
│   └── tests/             # IntegrationTests.cpp (10 end-to-end automated tests)
├── Database/              # Relational Database Scripts (3NF)
│   ├── schema.sql                 # MySQL 8.0+ DDL
│   ├── schema_sqlserver.sql       # Microsoft SQL Server (T-SQL) DDL
│   ├── seed.sql                   # Sample demo data
│   └── sample_students_import.csv # Sample CSV file for batch student import (FR05)
├── CMakeLists.txt
├── build_windows.bat
└── build_linux.sh
```

## Build & Run

### Windows (MinGW g++)
Run `build_windows.bat` or execute in PowerShell:
```powershell
g++ -std=c++17 -O2 -IBackend Backend/main.cpp Backend/core/*.cpp Backend/middlewares/*.cpp Backend/utils/*.cpp Backend/controllers/*.cpp Backend/routes/*.cpp -o attendance_server.exe -lws2_32
.\attendance_server.exe
```

### Linux / macOS
```bash
chmod +x build_linux.sh
./build_linux.sh
```

### CMake (Visual Studio / VS Code)
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Open **http://localhost:8080** in your browser.

### Default Demo Credentials
| Role          | Username | Password |
|---------------|----------|----------|
| Administrator | `admin`  | `123456` |
| Lecturer      | `gv001`  | `123456` |
| Student       | `SV001`  | `123456` |
| Student       | `SV002`  | `123456` |
| Student       | `SV003`  | `123456` |
