-- ============================================================================
-- STUDENT ATTENDANCE SYSTEM - RELATIONAL DATABASE SCHEMA (3NF)
-- Compatible with MySQL 8.0+ (notes provided for SQL Server adaptation)
-- Aligned with SRS v2.0 (FR01-FR16, NFR01-NFR10) & UML Class Diagram Final v2
-- ============================================================================

CREATE DATABASE IF NOT EXISTS student_attendance_db
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE student_attendance_db;

-- 1. Users: Authentication, hashed passwords, RBAC roles, lock status (FR01-FR03, NFR03)
CREATE TABLE Users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(20) NOT NULL CHECK (role IN ('Administrator', 'Lecturer', 'Student')),
    is_locked BOOLEAN NOT NULL DEFAULT FALSE,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 2. Administrators: System administrator profile
CREATE TABLE Administrators (
    admin_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT UNIQUE,
    admin_code VARCHAR(20) NOT NULL UNIQUE,
    full_name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE SET NULL
);

-- 3. Lecturers: Academic faculty profile
CREATE TABLE Lecturers (
    lecturer_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT UNIQUE,
    lecturer_code VARCHAR(20) NOT NULL UNIQUE,
    full_name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    department VARCHAR(100),
    FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE SET NULL
);

-- 4. Courses: Curriculum course catalog (FR06)
CREATE TABLE Courses (
    course_id INT AUTO_INCREMENT PRIMARY KEY,
    course_code VARCHAR(20) NOT NULL UNIQUE,
    course_name VARCHAR(100) NOT NULL,
    credits INT NOT NULL DEFAULT 3 CHECK (credits > 0)
);

-- 5. Classes: Class cohorts with assigned lecturer, schedule, and room (FR07, UC04)
CREATE TABLE Classes (
    class_id INT AUTO_INCREMENT PRIMARY KEY,
    class_code VARCHAR(20) NOT NULL UNIQUE,
    class_name VARCHAR(100) NOT NULL,
    academic_year VARCHAR(20),
    course_id INT NOT NULL,
    lecturer_id INT NOT NULL,
    schedule VARCHAR(100) NOT NULL,
    room VARCHAR(50) NOT NULL,
    FOREIGN KEY (course_id) REFERENCES Courses(course_id) ON DELETE RESTRICT,
    FOREIGN KEY (lecturer_id) REFERENCES Lecturers(lecturer_id) ON DELETE RESTRICT
);

-- 6. Students: Enrolled student profiles (FR04, FR05)
CREATE TABLE Students (
    student_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT UNIQUE,
    student_code VARCHAR(20) NOT NULL UNIQUE,
    full_name VARCHAR(100) NOT NULL,
    date_of_birth DATE,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone_number VARCHAR(15),
    class_id INT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE SET NULL,
    FOREIGN KEY (class_id) REFERENCES Classes(class_id) ON DELETE RESTRICT
);

-- 7. AttendanceSessions: Time-bound check-in sessions with code & GPS options (FR15, UC08)
CREATE TABLE AttendanceSessions (
    session_id INT AUTO_INCREMENT PRIMARY KEY,
    class_id INT NOT NULL,
    lecturer_id INT NOT NULL,
    session_date DATE NOT NULL,
    time_slot VARCHAR(30) NOT NULL,
    start_time TIME NOT NULL,
    end_time TIME NOT NULL,
    session_code VARCHAR(12) NOT NULL,
    require_location BOOLEAN NOT NULL DEFAULT FALSE,
    class_latitude DOUBLE NULL,
    class_longitude DOUBLE NULL,
    allowed_distance FLOAT NOT NULL DEFAULT 100.0,
    session_status VARCHAR(20) NOT NULL DEFAULT 'Open' CHECK (session_status IN ('Open', 'Closed')),
    CONSTRAINT UQ_Class_Date_Slot UNIQUE (class_id, session_date, time_slot),
    FOREIGN KEY (class_id) REFERENCES Classes(class_id) ON DELETE RESTRICT,
    FOREIGN KEY (lecturer_id) REFERENCES Lecturers(lecturer_id) ON DELETE RESTRICT
);

-- 8. AttendanceRecords: Student status, GPS verification & audit log (FR09, FR10, FR16, NFR09)
CREATE TABLE AttendanceRecords (
    record_id INT AUTO_INCREMENT PRIMARY KEY,
    session_id INT NOT NULL,
    student_id INT NOT NULL,
    old_status VARCHAR(20) NULL,
    status VARCHAR(20) NOT NULL CHECK (status IN ('Present', 'Absent', 'Late', 'Excused')),
    check_in_time DATETIME NULL,
    device_latitude DOUBLE NULL,
    device_longitude DOUBLE NULL,
    is_outside_area BOOLEAN NOT NULL DEFAULT FALSE,
    edit_reason VARCHAR(255) NULL,
    updated_by INT NULL,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT UQ_Session_Student UNIQUE (session_id, student_id),
    FOREIGN KEY (session_id) REFERENCES AttendanceSessions(session_id) ON DELETE CASCADE,
    FOREIGN KEY (student_id) REFERENCES Students(student_id) ON DELETE CASCADE,
    FOREIGN KEY (updated_by) REFERENCES Users(user_id) ON DELETE SET NULL
);
