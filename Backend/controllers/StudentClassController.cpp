#include "controllers/StudentClassController.h"
#include "core/JsonUtils.h"
#include "core/SecurityUtils.h"
#include <algorithm>
#include <cctype>
#include <sstream>

static std::string toLowerCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

StudentClassController::StudentClassController(DatabaseContext& database) : db(database) {}

HttpResponse StudentClassController::listStudents(const HttpRequest& req) {
    std::string keyword;
    auto qIt = req.queryParams.find("q");
    if (qIt != req.queryParams.end()) keyword = toLowerCopy(qIt->second);

    int filterClassId = 0;
    auto cIt = req.queryParams.find("class_id");
    if (cIt != req.queryParams.end() && !cIt->second.empty()) {
        try { filterClassId = std::stoi(cIt->second); } catch (...) {}
    }

    std::lock_guard<std::mutex> lock(db.mtx);
    std::vector<std::string> items;
    for (const auto& s : db.students) {
        if (filterClassId > 0 && s.classId != filterClassId) continue;
        if (!keyword.empty()) {
            std::string hay = toLowerCopy(s.studentCode + " " + s.fullName + " " + s.email);
            if (hay.find(keyword) == std::string::npos) continue;
        }
        ClassModel* cls = db.findClassById(s.classId);
        std::ostringstream oss;
        oss << "{\"student_id\":" << s.studentId
            << ",\"student_code\":" << JsonUtils::quote(s.studentCode)
            << ",\"full_name\":" << JsonUtils::quote(s.fullName)
            << ",\"date_of_birth\":" << JsonUtils::quote(s.dateOfBirth)
            << ",\"email\":" << JsonUtils::quote(s.email)
            << ",\"phone_number\":" << JsonUtils::quote(s.phoneNumber)
            << ",\"class_id\":" << s.classId
            << ",\"class_code\":" << JsonUtils::quote(cls ? cls->classCode : "") << "}";
        items.push_back(oss.str());
    }
    return HttpResponse::json(200, JsonUtils::joinArray(items));
}

HttpResponse StudentClassController::createStudent(const HttpRequest& req) {
    auto body = JsonUtils::parseFlatObject(req.body);
    std::string code = JsonUtils::getString(body, "student_code");
    std::string name = JsonUtils::getString(body, "full_name");
    std::string email = JsonUtils::getString(body, "email");
    int classId = JsonUtils::getInt(body, "class_id", 1);

    if (code.empty() || name.empty() || email.empty()) {
        return HttpResponse::error(400, "student_code, full_name, and email are required");
    }

    std::lock_guard<std::mutex> lock(db.mtx);
    // UC03 Alternative Flow 3a: Prevent duplicate student ID
    for (const auto& st : db.students) {
        if (st.studentCode == code) return HttpResponse::error(409, "Student ID already exists");
    }
    if (!db.findClassById(classId)) return HttpResponse::error(400, "Invalid class_id");

    int uid = db.nextUserId++;
    db.users.emplace_back(uid, code, SecurityUtils::hashPassword("123456"), "Student", false, SecurityUtils::getCurrentDateTimeString());

    Student s;
    s.studentId = db.nextStudentId++;
    s.userId = uid;
    s.studentCode = code;
    s.fullName = name;
    s.dateOfBirth = JsonUtils::getString(body, "date_of_birth", "2006-01-01");
    s.email = email;
    s.phoneNumber = JsonUtils::getString(body, "phone_number");
    s.classId = classId;
    db.students.push_back(s);

    return HttpResponse::json(201, "{\"message\":\"Student created\",\"student_id\":" + std::to_string(s.studentId) + "}");
}

HttpResponse StudentClassController::deleteStudent(int studentId) {
    std::lock_guard<std::mutex> lock(db.mtx);
    auto it = std::remove_if(db.students.begin(), db.students.end(),
                             [studentId](const Student& s) { return s.studentId == studentId; });
    if (it == db.students.end()) return HttpResponse::error(404, "Student not found");
    db.students.erase(it, db.students.end());
    return HttpResponse::json(200, "{\"message\":\"Student removed\"}");
}

HttpResponse StudentClassController::importStudentsCsv(const HttpRequest& req) {
    // Accepts raw CSV text: student_code,full_name,email,class_id
    std::istringstream iss(req.body);
    std::string line;
    int imported = 0;
    bool firstLine = true;

    std::lock_guard<std::mutex> lock(db.mtx);
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        if (firstLine && line.find("student_code") != std::string::npos) {
            firstLine = false;
            continue;
        }
        firstLine = false;

        std::stringstream ss(line);
        std::string code, name, email, classIdStr;
        std::getline(ss, code, ',');
        std::getline(ss, name, ',');
        std::getline(ss, email, ',');
        std::getline(ss, classIdStr, ',');
        if (code.empty() || name.empty()) continue;

        bool duplicate = false;
        for (const auto& s : db.students) if (s.studentCode == code) duplicate = true;
        if (duplicate) continue;

        int classId = 1;
        try { if (!classIdStr.empty()) classId = std::stoi(classIdStr); } catch (...) {}
        int uid = db.nextUserId++;
        db.users.emplace_back(uid, code, SecurityUtils::hashPassword("123456"), "Student", false, SecurityUtils::getCurrentDateTimeString());

        Student st;
        st.studentId = db.nextStudentId++;
        st.userId = uid;
        st.studentCode = code;
        st.fullName = name;
        st.email = email.empty() ? (code + "@university.edu") : email;
        st.classId = classId;
        db.students.push_back(st);
        ++imported;
    }
    return HttpResponse::json(201, "{\"imported\":" + std::to_string(imported) + "}");
}

HttpResponse StudentClassController::listClasses() {
    std::lock_guard<std::mutex> lock(db.mtx);
    std::vector<std::string> items;
    for (const auto& c : db.classes) {
        Course* crs = db.findCourseById(c.courseId);
        std::string lecName;
        for (const auto& l : db.lecturers) if (l.lecturerId == c.lecturerId) lecName = l.fullName;
        std::ostringstream oss;
        oss << "{\"class_id\":" << c.classId
            << ",\"class_code\":" << JsonUtils::quote(c.classCode)
            << ",\"class_name\":" << JsonUtils::quote(c.className)
            << ",\"academic_year\":" << JsonUtils::quote(c.academicYear)
            << ",\"course_id\":" << c.courseId
            << ",\"course_name\":" << JsonUtils::quote(crs ? crs->courseName : "")
            << ",\"lecturer_id\":" << c.lecturerId
            << ",\"lecturer_name\":" << JsonUtils::quote(lecName)
            << ",\"schedule\":" << JsonUtils::quote(c.schedule)
            << ",\"room\":" << JsonUtils::quote(c.room) << "}";
        items.push_back(oss.str());
    }
    return HttpResponse::json(200, JsonUtils::joinArray(items));
}

HttpResponse StudentClassController::createClass(const HttpRequest& req) {
    auto body = JsonUtils::parseFlatObject(req.body);
    ClassModel cls;
    cls.classCode = JsonUtils::getString(body, "class_code");
    cls.className = JsonUtils::getString(body, "class_name");
    cls.academicYear = JsonUtils::getString(body, "academic_year", "2024-2028");
    cls.courseId = JsonUtils::getInt(body, "course_id", 1);
    cls.lecturerId = JsonUtils::getInt(body, "lecturer_id", 1);
    cls.schedule = JsonUtils::getString(body, "schedule");
    cls.room = JsonUtils::getString(body, "room");

    if (cls.classCode.empty() || cls.className.empty() || cls.schedule.empty() || cls.room.empty()) {
        return HttpResponse::error(400, "class_code, class_name, schedule, and room are required");
    }

    std::lock_guard<std::mutex> lock(db.mtx);
    // UC04 Alternative Flow 2a: Lecturer or room schedule conflict warning
    for (const auto& existing : db.classes) {
        if (existing.classCode == cls.classCode) {
            return HttpResponse::error(409, "Class code already exists");
        }
        if (existing.schedule == cls.schedule &&
            (existing.lecturerId == cls.lecturerId || existing.room == cls.room)) {
            return HttpResponse::error(409, "Schedule conflict: Lecturer or room is already occupied at that time");
        }
    }

    cls.classId = db.nextClassId++;
    db.classes.push_back(cls);
    return HttpResponse::json(201, "{\"message\":\"Class created\",\"class_id\":" + std::to_string(cls.classId) + "}");
}
