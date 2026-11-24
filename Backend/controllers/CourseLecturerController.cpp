#include "controllers/CourseLecturerController.h"
#include "core/JsonUtils.h"
#include <sstream>

CourseLecturerController::CourseLecturerController(DatabaseContext& database) : db(database) {}

HttpResponse CourseLecturerController::listCourses() {
    std::lock_guard<std::mutex> lock(db.mtx);
    std::vector<std::string> items;
    for (const auto& c : db.courses) {
        std::ostringstream oss;
        oss << "{\"course_id\":" << c.courseId
            << ",\"course_code\":" << JsonUtils::quote(c.courseCode)
            << ",\"course_name\":" << JsonUtils::quote(c.courseName)
            << ",\"credits\":" << c.credits << "}";
        items.push_back(oss.str());
    }
    return HttpResponse::json(200, JsonUtils::joinArray(items));
}

HttpResponse CourseLecturerController::createCourse(const HttpRequest& req) {
    auto body = JsonUtils::parseFlatObject(req.body);
    std::string code = JsonUtils::getString(body, "course_code");
    std::string name = JsonUtils::getString(body, "course_name");
    int credits = JsonUtils::getInt(body, "credits", 3);

    if (code.empty() || name.empty() || credits <= 0) {
        return HttpResponse::error(400, "Valid course_code, course_name, and positive credits are required");
    }

    std::lock_guard<std::mutex> lock(db.mtx);
    for (const auto& c : db.courses) {
        if (c.courseCode == code) return HttpResponse::error(409, "Course code already exists");
    }
    Course course{db.nextCourseId++, code, name, credits};
    db.courses.push_back(course);
    return HttpResponse::json(201, "{\"message\":\"Course created\",\"course_id\":" + std::to_string(course.courseId) + "}");
}

HttpResponse CourseLecturerController::listLecturers() {
    std::lock_guard<std::mutex> lock(db.mtx);
    std::vector<std::string> items;
    for (const auto& l : db.lecturers) {
        std::ostringstream oss;
        oss << "{\"lecturer_id\":" << l.lecturerId
            << ",\"lecturer_code\":" << JsonUtils::quote(l.lecturerCode)
            << ",\"full_name\":" << JsonUtils::quote(l.fullName)
            << ",\"email\":" << JsonUtils::quote(l.email)
            << ",\"department\":" << JsonUtils::quote(l.department) << "}";
        items.push_back(oss.str());
    }
    return HttpResponse::json(200, JsonUtils::joinArray(items));
}

HttpResponse CourseLecturerController::viewMyClasses(const AuthSession& session) {
    std::lock_guard<std::mutex> lock(db.mtx);
    Lecturer* lec = db.findLecturerByUserId(session.userId);
    if (!lec) return HttpResponse::error(404, "Lecturer profile not found");

    std::vector<std::string> items;
    for (const auto& c : db.classes) {
        if (c.lecturerId != lec->lecturerId) continue;
        Course* crs = db.findCourseById(c.courseId);
        std::ostringstream oss;
        oss << "{\"class_id\":" << c.classId
            << ",\"class_code\":" << JsonUtils::quote(c.classCode)
            << ",\"class_name\":" << JsonUtils::quote(c.className)
            << ",\"course_name\":" << JsonUtils::quote(crs ? crs->courseName : "")
            << ",\"schedule\":" << JsonUtils::quote(c.schedule)
            << ",\"room\":" << JsonUtils::quote(c.room) << "}";
        items.push_back(oss.str());
    }
    return HttpResponse::json(200, JsonUtils::joinArray(items));
}
