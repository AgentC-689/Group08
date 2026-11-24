-- ============================================================================
-- STUDENT ATTENDANCE SYSTEM - INITIAL DEMO SEED DATA (Standard SQL)
-- Default password for all demo accounts: "123456" (stored as hashed value)
-- ============================================================================

INSERT INTO Users (username, password_hash, role, is_locked) VALUES
('admin', 'hash$demo_salted_password_123456', 'Administrator', 0),
('gv001', 'hash$demo_salted_password_123456', 'Lecturer', 0),
('SV001', 'hash$demo_salted_password_123456', 'Student', 0),
('SV002', 'hash$demo_salted_password_123456', 'Student', 0),
('SV003', 'hash$demo_salted_password_123456', 'Student', 0);

INSERT INTO Administrators (user_id, admin_code, full_name, email) VALUES
(1, 'AD001', 'System Administrator', 'admin@university.edu');

INSERT INTO Lecturers (user_id, lecturer_code, full_name, email, department) VALUES
(2, 'GV001', 'Dr. Tran Minh Khoi', 'khoi.tm@university.edu', 'Information Technology');

INSERT INTO Courses (course_code, course_name, credits) VALUES
('CS101', 'Software Engineering', 3),
('CS204', 'Database Systems', 3);

INSERT INTO Classes (class_code, class_name, academic_year, course_id, lecturer_id, schedule, room) VALUES
('CNTT01', 'Information Technology 01', '2024-2028', 1, 1, 'Mon 07:00-09:30', 'A201'),
('CNTT02', 'Information Technology 02', '2024-2028', 2, 1, 'Wed 09:45-12:15', 'B105');

INSERT INTO Students (user_id, student_code, full_name, date_of_birth, email, phone_number, class_id) VALUES
(3, 'SV001', 'Nguyen Van A', '2006-01-28', 'sv001@university.edu', '0901234567', 1),
(4, 'SV002', 'Tran Van B',   '2006-03-15', 'sv002@university.edu', '0901234568', 1),
(5, 'SV003', 'Le Van C',     '2006-05-20', 'sv003@university.edu', '0901234569', 1);

INSERT INTO AttendanceSessions (class_id, lecturer_id, session_date, time_slot, start_time, end_time, session_code, require_location, class_latitude, class_longitude, allowed_distance, session_status) VALUES
(1, 1, '2026-11-23', 'Slot 1', '07:00:00', '07:30:00', 'WEB123', 1, 10.8037, 106.7168, 100.0, 'Open');

INSERT INTO AttendanceRecords (session_id, student_id, status, check_in_time, device_latitude, device_longitude, is_outside_area, updated_by) VALUES
(1, 1, 'Present', '2026-11-23 07:05:00', 10.8037, 106.7168, 0, 3),
(1, 2, 'Absent', NULL, NULL, NULL, 0, 2);
