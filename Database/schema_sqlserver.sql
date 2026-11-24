-- ============================================================================
-- STUDENT ATTENDANCE SYSTEM - MICROSOFT SQL SERVER (T-SQL) SCHEMA (3NF)
-- Aligned with SRS v2.0 (FR01-FR16, NFR01-NFR10) & UML Class Diagram Final v2
-- ============================================================================

IF DB_ID(N'StudentAttendanceDB') IS NULL
BEGIN
    CREATE DATABASE StudentAttendanceDB;
END
GO

USE StudentAttendanceDB;
GO

-- 1. Users
CREATE TABLE Users (
    user_id INT IDENTITY(1,1) PRIMARY KEY,
    username NVARCHAR(50) NOT NULL UNIQUE,
    password_hash NVARCHAR(255) NOT NULL,
    role NVARCHAR(20) NOT NULL CHECK (role IN ('Administrator', 'Lecturer', 'Student')),
    is_locked BIT NOT NULL DEFAULT 0,
    created_at DATETIME2 NOT NULL DEFAULT SYSDATETIME()
);
GO

-- 2. Administrators
CREATE TABLE Administrators (
    admin_id INT IDENTITY(1,1) PRIMARY KEY,
    user_id INT UNIQUE NULL,
    admin_code NVARCHAR(20) NOT NULL UNIQUE,
    full_name NVARCHAR(100) NOT NULL,
    email NVARCHAR(100) NOT NULL UNIQUE,
    CONSTRAINT FK_Administrators_Users FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE SET NULL
);
GO

-- 3. Lecturers
CREATE TABLE Lecturers (
    lecturer_id INT IDENTITY(1,1) PRIMARY KEY,
    user_id INT UNIQUE NULL,
    lecturer_code NVARCHAR(20) NOT NULL UNIQUE,
    full_name NVARCHAR(100) NOT NULL,
    email NVARCHAR(100) NOT NULL UNIQUE,
    department NVARCHAR(100) NULL,
    CONSTRAINT FK_Lecturers_Users FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE SET NULL
);
GO

-- 4. Courses
CREATE TABLE Courses (
    course_id INT IDENTITY(1,1) PRIMARY KEY,
    course_code NVARCHAR(20) NOT NULL UNIQUE,
    course_name NVARCHAR(100) NOT NULL,
    credits INT NOT NULL DEFAULT 3 CHECK (credits > 0)
);
GO

-- 5. Classes
CREATE TABLE Classes (
    class_id INT IDENTITY(1,1) PRIMARY KEY,
    class_code NVARCHAR(20) NOT NULL UNIQUE,
    class_name NVARCHAR(100) NOT NULL,
    academic_year NVARCHAR(20) NULL,
    course_id INT NOT NULL,
    lecturer_id INT NOT NULL,
    schedule NVARCHAR(100) NOT NULL,
    room NVARCHAR(50) NOT NULL,
    CONSTRAINT FK_Classes_Courses FOREIGN KEY (course_id) REFERENCES Courses(course_id) ON DELETE NO ACTION,
    CONSTRAINT FK_Classes_Lecturers FOREIGN KEY (lecturer_id) REFERENCES Lecturers(lecturer_id) ON DELETE NO ACTION
);
GO

-- 6. Students
CREATE TABLE Students (
    student_id INT IDENTITY(1,1) PRIMARY KEY,
    user_id INT UNIQUE NULL,
    student_code NVARCHAR(20) NOT NULL UNIQUE,
    full_name NVARCHAR(100) NOT NULL,
    date_of_birth DATE NULL,
    email NVARCHAR(100) NOT NULL UNIQUE,
    phone_number NVARCHAR(15) NULL,
    class_id INT NOT NULL,
    CONSTRAINT FK_Students_Users FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE SET NULL,
    CONSTRAINT FK_Students_Classes FOREIGN KEY (class_id) REFERENCES Classes(class_id) ON DELETE NO ACTION
);
GO

-- 7. AttendanceSessions
CREATE TABLE AttendanceSessions (
    session_id INT IDENTITY(1,1) PRIMARY KEY,
    class_id INT NOT NULL,
    lecturer_id INT NOT NULL,
    session_date DATE NOT NULL,
    time_slot NVARCHAR(30) NOT NULL,
    start_time TIME NOT NULL,
    end_time TIME NOT NULL,
    session_code NVARCHAR(12) NOT NULL,
    require_location BIT NOT NULL DEFAULT 0,
    class_latitude FLOAT NULL,
    class_longitude FLOAT NULL,
    allowed_distance FLOAT NOT NULL DEFAULT 100.0,
    session_status NVARCHAR(20) NOT NULL DEFAULT 'Open' CHECK (session_status IN ('Open', 'Closed')),
    CONSTRAINT UQ_Class_Date_Slot UNIQUE (class_id, session_date, time_slot),
    CONSTRAINT CK_Session_Time CHECK (end_time > start_time),
    CONSTRAINT FK_Sessions_Classes FOREIGN KEY (class_id) REFERENCES Classes(class_id) ON DELETE NO ACTION,
    CONSTRAINT FK_Sessions_Lecturers FOREIGN KEY (lecturer_id) REFERENCES Lecturers(lecturer_id) ON DELETE NO ACTION
);
GO

-- 8. AttendanceRecords
CREATE TABLE AttendanceRecords (
    record_id INT IDENTITY(1,1) PRIMARY KEY,
    session_id INT NOT NULL,
    student_id INT NOT NULL,
    old_status NVARCHAR(20) NULL,
    status NVARCHAR(20) NOT NULL CHECK (status IN ('Present', 'Absent', 'Late', 'Excused')),
    check_in_time DATETIME2 NULL,
    device_latitude FLOAT NULL,
    device_longitude FLOAT NULL,
    is_outside_area BIT NOT NULL DEFAULT 0,
    edit_reason NVARCHAR(255) NULL,
    updated_by INT NULL,
    updated_at DATETIME2 NOT NULL DEFAULT SYSDATETIME(),
    CONSTRAINT UQ_Session_Student UNIQUE (session_id, student_id),
    CONSTRAINT FK_Records_Sessions FOREIGN KEY (session_id) REFERENCES AttendanceSessions(session_id) ON DELETE CASCADE,
    CONSTRAINT FK_Records_Students FOREIGN KEY (student_id) REFERENCES Students(student_id) ON DELETE CASCADE,
    CONSTRAINT FK_Records_Users FOREIGN KEY (updated_by) REFERENCES Users(user_id) ON DELETE SET NULL
);
GO
