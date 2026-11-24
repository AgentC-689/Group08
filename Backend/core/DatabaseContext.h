#pragma once
#include "models/User.h"
#include "models/Administrator.h"
#include "models/Lecturer.h"
#include "models/Course.h"
#include "models/ClassModel.h"
#include "models/Student.h"
#include "models/AttendanceSession.h"
#include "models/AttendanceRecord.h"
#include <mutex>
#include <vector>

// Thread-safe Data Access Layer (Repository) mirroring the 8-table 3NF schema.
// Uses in-memory relational tables seeded with demo data for immediate execution;
// swap each method body with MySQL Connector/C++ or ODBC calls for SQL persistence.
class DatabaseContext {
public:
    std::mutex mtx;

    std::vector<User> users;
    std::vector<Administrator> admins;
    std::vector<Lecturer> lecturers;
    std::vector<Course> courses;
    std::vector<ClassModel> classes;
    std::vector<Student> students;
    std::vector<AttendanceSession> sessions;
    std::vector<AttendanceRecord> records;

    int nextUserId = 1;
    int nextAdminId = 1;
    int nextLecturerId = 1;
    int nextCourseId = 1;
    int nextClassId = 1;
    int nextStudentId = 1;
    int nextSessionId = 1;
    int nextRecordId = 1;

    DatabaseContext();
    void seedDemoData();

    User* findUserByUsername(const std::string& username);
    User* findUserById(int userId);
    Lecturer* findLecturerByUserId(int userId);
    Student* findStudentByUserId(int userId);
    Student* findStudentById(int studentId);
    ClassModel* findClassById(int classId);
    Course* findCourseById(int courseId);
    AttendanceSession* findSessionById(int sessionId);
    AttendanceRecord* findRecordById(int recordId);
    AttendanceRecord* findRecordBySessionAndStudent(int sessionId, int studentId);
};
