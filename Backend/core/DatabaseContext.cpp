#include "core/DatabaseContext.h"
#include "core/SecurityUtils.h"

DatabaseContext::DatabaseContext() {
    seedDemoData();
}

void DatabaseContext::seedDemoData() {
    const std::string now = SecurityUtils::getCurrentDateTimeString();
    const std::string today = SecurityUtils::getCurrentDateString();
    const std::string defaultHash = SecurityUtils::hashPassword("123456");

    // 1. Seed Administrator (admin / 123456)
    users.emplace_back(nextUserId++, "admin", defaultHash, "Administrator", false, now);
    Administrator adm;
    adm.adminId = nextAdminId++;
    adm.userId = 1;
    adm.adminCode = "AD001";
    adm.fullName = "System Administrator";
    adm.email = "admin@university.edu";
    admins.push_back(adm);

    // 2. Seed Lecturer (gv001 / 123456)
    users.emplace_back(nextUserId++, "gv001", defaultHash, "Lecturer", false, now);
    Lecturer lec;
    lec.lecturerId = nextLecturerId++;
    lec.userId = 2;
    lec.lecturerCode = "GV001";
    lec.fullName = "Dr. Tran Minh Khoi";
    lec.email = "khoi.tm@university.edu";
    lec.department = "Information Technology";
    lecturers.push_back(lec);

    // 3. Seed Courses
    Course c1{nextCourseId++, "CS101", "Software Engineering", 3};
    Course c2{nextCourseId++, "CS204", "Database Systems", 3};
    courses.push_back(c1);
    courses.push_back(c2);

    // 4. Seed Classes (matching Figma sample CNTT01, CNTT02)
    ClassModel cls1{nextClassId++, "CNTT01", "Information Technology 01", "2024-2028", 1, 1, "Mon 07:00-09:30", "A201"};
    ClassModel cls2{nextClassId++, "CNTT02", "Information Technology 02", "2024-2028", 2, 1, "Wed 09:45-12:15", "B105"};
    classes.push_back(cls1);
    classes.push_back(cls2);

    // 5. Seed Students (SV001, SV002, SV003 / 123456)
    struct SeedStudent { std::string code; std::string name; std::string email; int classId; };
    std::vector<SeedStudent> seedList = {
        {"SV001", "Nguyen Van A", "sv001@university.edu", 1},
        {"SV002", "Tran Van B",   "sv002@university.edu", 1},
        {"SV003", "Le Van C",     "sv003@university.edu", 1}
    };
    for (const auto& item : seedList) {
        int uid = nextUserId++;
        users.emplace_back(uid, item.code, defaultHash, "Student", false, now);
        Student st;
        st.studentId = nextStudentId++;
        st.userId = uid;
        st.studentCode = item.code;
        st.fullName = item.name;
        st.dateOfBirth = "2006-01-28";
        st.email = item.email;
        st.phoneNumber = "0901234567";
        st.classId = item.classId;
        students.push_back(st);
    }

    // 6. Seed an Open Attendance Session for today (00:00-23:59 for easy demo check-in)
    AttendanceSession s1;
    s1.sessionId = nextSessionId++;
    s1.classId = 1;
    s1.lecturerId = 1;
    s1.sessionDate = today;
    s1.timeSlot = "Slot 1";
    s1.startTime = "00:00";
    s1.endTime = "23:59";
    s1.sessionCode = "WEB123";
    s1.requireLocation = true;
    s1.classLatitude = 10.8037;
    s1.classLongitude = 106.7168;
    s1.allowedDistance = 100.0;
    s1.sessionStatus = "Open";
    sessions.push_back(s1);

    // 7. Seed Initial Attendance Records
    AttendanceRecord r1;
    r1.recordId = nextRecordId++;
    r1.sessionId = 1;
    r1.studentId = 1;
    r1.status = "Present";
    r1.checkInTime = now;
    r1.deviceLatitude = 10.8037;
    r1.deviceLongitude = 106.7168;
    r1.hasLocation = true;
    r1.isOutsideArea = false;
    r1.updatedBy = 3;
    r1.updatedAt = now;
    records.push_back(r1);

    AttendanceRecord r2;
    r2.recordId = nextRecordId++;
    r2.sessionId = 1;
    r2.studentId = 2;
    r2.status = "Absent";
    r2.updatedBy = 2;
    r2.updatedAt = now;
    records.push_back(r2);
}

User* DatabaseContext::findUserByUsername(const std::string& username) {
    for (auto& u : users) if (u.username == username) return &u;
    return nullptr;
}

User* DatabaseContext::findUserById(int userId) {
    for (auto& u : users) if (u.userId == userId) return &u;
    return nullptr;
}

Lecturer* DatabaseContext::findLecturerByUserId(int userId) {
    for (auto& l : lecturers) if (l.userId == userId) return &l;
    return nullptr;
}

Student* DatabaseContext::findStudentByUserId(int userId) {
    for (auto& s : students) if (s.userId == userId) return &s;
    return nullptr;
}

Student* DatabaseContext::findStudentById(int studentId) {
    for (auto& s : students) if (s.studentId == studentId) return &s;
    return nullptr;
}

ClassModel* DatabaseContext::findClassById(int classId) {
    for (auto& c : classes) if (c.classId == classId) return &c;
    return nullptr;
}

Course* DatabaseContext::findCourseById(int courseId) {
    for (auto& c : courses) if (c.courseId == courseId) return &c;
    return nullptr;
}

AttendanceSession* DatabaseContext::findSessionById(int sessionId) {
    for (auto& s : sessions) if (s.sessionId == sessionId) return &s;
    return nullptr;
}

AttendanceRecord* DatabaseContext::findRecordById(int recordId) {
    for (auto& r : records) if (r.recordId == recordId) return &r;
    return nullptr;
}

AttendanceRecord* DatabaseContext::findRecordBySessionAndStudent(int sessionId, int studentId) {
    for (auto& r : records) {
        if (r.sessionId == sessionId && r.studentId == studentId) return &r;
    }
    return nullptr;
}
