#include "controllers/AttendanceController.h"
#include "config/Config.h"
#include "core/JsonUtils.h"
#include "core/SecurityUtils.h"
#include "utils/GeoUtils.h"
#include <sstream>

AttendanceController::AttendanceController(DatabaseContext& database) : db(database) {}

HttpResponse AttendanceController::listSessions(const AuthSession& session) {
    std::lock_guard<std::mutex> lock(db.mtx);
    int filterClassId = 0;
    int filterLecturerId = 0;

    if (session.role == "Student") {
        Student* st = db.findStudentByUserId(session.userId);
        if (st) filterClassId = st->classId;
    } else if (session.role == "Lecturer") {
        Lecturer* lec = db.findLecturerByUserId(session.userId);
        if (lec) filterLecturerId = lec->lecturerId;
    }

    std::vector<std::string> items;
    for (const auto& s : db.sessions) {
        if (filterClassId > 0 && s.classId != filterClassId) continue;
        if (filterLecturerId > 0 && s.lecturerId != filterLecturerId) continue;
        ClassModel* cls = db.findClassById(s.classId);

        std::ostringstream oss;
        oss << "{\"session_id\":" << s.sessionId
            << ",\"class_id\":" << s.classId
            << ",\"class_code\":" << JsonUtils::quote(cls ? cls->classCode : "")
            << ",\"session_date\":" << JsonUtils::quote(s.sessionDate)
            << ",\"time_slot\":" << JsonUtils::quote(s.timeSlot)
            << ",\"start_time\":" << JsonUtils::quote(s.startTime)
            << ",\"end_time\":" << JsonUtils::quote(s.endTime)
            << ",\"require_location\":" << (s.requireLocation ? "true" : "false")
            << ",\"allowed_distance\":" << s.allowedDistance
            << ",\"session_status\":" << JsonUtils::quote(s.sessionStatus);
        // Hide session_code from students so they must obtain it from the lecturer in class
        if (session.role != "Student") {
            oss << ",\"session_code\":" << JsonUtils::quote(s.sessionCode);
        }
        oss << "}";
        items.push_back(oss.str());
    }
    return HttpResponse::json(200, JsonUtils::joinArray(items));
}

HttpResponse AttendanceController::createSession(const HttpRequest& req, const AuthSession& session) {
    auto body = JsonUtils::parseFlatObject(req.body);

    std::lock_guard<std::mutex> lock(db.mtx);
    Lecturer* lec = db.findLecturerByUserId(session.userId);
    if (!lec) return HttpResponse::error(403, "Lecturer profile required");

    AttendanceSession s;
    s.classId = JsonUtils::getInt(body, "class_id", 0);
    s.lecturerId = lec->lecturerId;
    s.sessionDate = JsonUtils::getString(body, "session_date", SecurityUtils::getCurrentDateString());
    s.timeSlot = JsonUtils::getString(body, "time_slot", "Slot 1");
    s.startTime = JsonUtils::getString(body, "start_time");
    s.endTime = JsonUtils::getString(body, "end_time");
    s.requireLocation = JsonUtils::getBool(body, "require_location", false);
    s.classLatitude = JsonUtils::getDouble(body, "class_latitude", 0.0);
    s.classLongitude = JsonUtils::getDouble(body, "class_longitude", 0.0);
    s.allowedDistance = JsonUtils::getDouble(body, "allowed_distance", Config::DEFAULT_ALLOWED_DISTANCE_METERS);
    s.sessionStatus = "Open";

    ClassModel* cls = db.findClassById(s.classId);
    if (!cls || cls->lecturerId != lec->lecturerId) {
        return HttpResponse::error(403, "You are not assigned to teach this class");
    }
    // UC08 Alternative Flow 3a: End time must be later than start time
    if (!s.hasValidTimeRange()) {
        return HttpResponse::error(400, "End time must be later than start time");
    }
    // FR15 / UC08 Alternative Flow 3a: Prevent two sessions for the same class and time slot
    for (const auto& existing : db.sessions) {
        if (existing.classId == s.classId &&
            existing.sessionDate == s.sessionDate &&
            existing.timeSlot == s.timeSlot) {
            return HttpResponse::error(409, "A session already exists for this class and time slot");
        }
    }

    s.sessionId = db.nextSessionId++;
    s.sessionCode = SecurityUtils::generateSessionCode(6);
    db.sessions.push_back(s);

    std::ostringstream oss;
    oss << "{\"message\":\"Attendance session created\""
        << ",\"session_id\":" << s.sessionId
        << ",\"session_code\":" << JsonUtils::quote(s.sessionCode) << "}";
    return HttpResponse::json(201, oss.str());
}

HttpResponse AttendanceController::closeSessionAndMarkAbsent(int sessionId, const AuthSession& session) {
    std::lock_guard<std::mutex> lock(db.mtx);
    AttendanceSession* sess = db.findSessionById(sessionId);
    if (!sess) return HttpResponse::error(404, "Session not found");

    int markedAbsent = 0;
    const std::string now = SecurityUtils::getCurrentDateTimeString();
    // FR16: Students who did not check in when the session ends are automatically marked Absent
    for (const auto& st : db.students) {
        if (st.classId != sess->classId) continue;
        if (!db.findRecordBySessionAndStudent(sessionId, st.studentId)) {
            AttendanceRecord rec;
            rec.recordId = db.nextRecordId++;
            rec.sessionId = sessionId;
            rec.studentId = st.studentId;
            rec.status = "Absent";
            rec.updatedBy = session.userId;
            rec.updatedAt = now;
            db.records.push_back(rec);
            ++markedAbsent;
        }
    }
    sess->sessionStatus = "Closed";
    return HttpResponse::json(200, "{\"message\":\"Session closed\",\"auto_marked_absent\":" + std::to_string(markedAbsent) + "}");
}

HttpResponse AttendanceController::getSessionRoster(int sessionId) {
    std::lock_guard<std::mutex> lock(db.mtx);
    AttendanceSession* sess = db.findSessionById(sessionId);
    if (!sess) return HttpResponse::error(404, "Session not found");

    std::vector<std::string> items;
    for (const auto& st : db.students) {
        if (st.classId != sess->classId) continue;
        AttendanceRecord* rec = db.findRecordBySessionAndStudent(sessionId, st.studentId);
        std::ostringstream oss;
        oss << "{\"student_id\":" << st.studentId
            << ",\"student_code\":" << JsonUtils::quote(st.studentCode)
            << ",\"full_name\":" << JsonUtils::quote(st.fullName)
            << ",\"record_id\":" << (rec ? rec->recordId : 0)
            << ",\"status\":" << JsonUtils::quote(rec ? rec->status : "Unmarked")
            << ",\"old_status\":" << JsonUtils::quote(rec ? rec->oldStatus : "")
            << ",\"is_outside_area\":" << (rec && rec->isOutsideArea ? "true" : "false")
            << ",\"edit_reason\":" << JsonUtils::quote(rec ? rec->editReason : "") << "}";
        items.push_back(oss.str());
    }
    return HttpResponse::json(200, JsonUtils::joinArray(items));
}

HttpResponse AttendanceController::takeAttendance(int sessionId, const HttpRequest& req, const AuthSession& session) {
    auto body = JsonUtils::parseFlatObject(req.body);
    int studentId = JsonUtils::getInt(body, "student_id", 0);
    std::string status = JsonUtils::getString(body, "status");

    if (status != "Present" && status != "Absent" && status != "Late" && status != "Excused") {
        return HttpResponse::error(400, "Invalid status. Allowed: Present, Absent, Late, Excused");
    }

    std::lock_guard<std::mutex> lock(db.mtx);
    if (!db.findSessionById(sessionId)) return HttpResponse::error(404, "Session not found");
    if (!db.findStudentById(studentId)) return HttpResponse::error(404, "Student not found");

    const std::string now = SecurityUtils::getCurrentDateTimeString();
    AttendanceRecord* existing = db.findRecordBySessionAndStudent(sessionId, studentId);
    if (existing) {
        existing->oldStatus = existing->status;
        existing->status = status;
        existing->updatedBy = session.userId;
        existing->updatedAt = now;
        return HttpResponse::json(200, "{\"message\":\"Attendance updated\",\"record_id\":" + std::to_string(existing->recordId) + "}");
    }

    AttendanceRecord rec;
    rec.recordId = db.nextRecordId++;
    rec.sessionId = sessionId;
    rec.studentId = studentId;
    rec.status = status;
    rec.checkInTime = now;
    rec.updatedBy = session.userId;
    rec.updatedAt = now;
    db.records.push_back(rec);
    return HttpResponse::json(201, "{\"message\":\"Attendance saved\",\"record_id\":" + std::to_string(rec.recordId) + "}");
}

HttpResponse AttendanceController::studentCheckIn(const HttpRequest& req, const AuthSession& session) {
    auto body = JsonUtils::parseFlatObject(req.body);
    int sessionId = JsonUtils::getInt(body, "session_id", 0);
    std::string code = JsonUtils::getString(body, "session_code");
    std::string curDate = JsonUtils::getString(body, "current_date", SecurityUtils::getCurrentDateString());
    std::string curTime = JsonUtils::getString(body, "current_time", SecurityUtils::getCurrentTimeString());

    std::lock_guard<std::mutex> lock(db.mtx);
    Student* st = db.findStudentByUserId(session.userId);
    if (!st) return HttpResponse::error(403, "Student profile not found");

    AttendanceSession* sess = db.findSessionById(sessionId);
    if (!sess) return HttpResponse::error(404, "Attendance session not found");
    if (sess->classId != st->classId) return HttpResponse::error(403, "You do not belong to this class");

    // UC09 Alternative Flow 4a: Wrong session code
    if (code != sess->sessionCode) return HttpResponse::error(400, "Wrong session code");

    // UC09 Alternative Flow 4b: Refused before start time or after end time
    if (!sess->isOpenAt(curDate, curTime)) {
        return HttpResponse::error(400, "Check-in refused: session is not open at the current time");
    }

    // UC09 Alternative Flow 4c: Already checked in
    if (AttendanceRecord* existing = db.findRecordBySessionAndStudent(sessionId, st->studentId)) {
        return HttpResponse::json(200, "{\"message\":\"Already checked in\",\"status\":" + JsonUtils::quote(existing->status) + "}");
    }

    bool isOutside = false;
    bool hasLat = body.count("device_latitude") > 0 && body["device_latitude"] != "null";
    bool hasLng = body.count("device_longitude") > 0 && body["device_longitude"] != "null";
    double devLat = JsonUtils::getDouble(body, "device_latitude", 0.0);
    double devLng = JsonUtils::getDouble(body, "device_longitude", 0.0);

    if (sess->requireLocation) {
        // UC09 Alternative Flow 4e: Location permission required
        if (!hasLat || !hasLng) {
            return HttpResponse::error(400, "Location access is required for this session");
        }
        // UC09 Alternative Flow 4d: Outside allowed distance -> save & flag "Outside class area"
        if (!GeoUtils::isWithinAllowedRadius(sess->classLatitude, sess->classLongitude, devLat, devLng, sess->allowedDistance)) {
            isOutside = true;
        }
    }

    const std::string now = SecurityUtils::getCurrentDateTimeString();
    AttendanceRecord rec;
    rec.recordId = db.nextRecordId++;
    rec.sessionId = sessionId;
    rec.studentId = st->studentId;
    rec.status = "Present";
    rec.checkInTime = now;
    rec.deviceLatitude = devLat;
    rec.deviceLongitude = devLng;
    rec.hasLocation = (hasLat && hasLng);
    rec.isOutsideArea = isOutside;
    rec.updatedBy = session.userId;
    rec.updatedAt = now;
    db.records.push_back(rec);

    std::string msg = isOutside
        ? "Check-in saved, marked 'Outside class area' for lecturer review"
        : "Check-in successful: Present";
    std::ostringstream oss;
    oss << "{\"message\":" << JsonUtils::quote(msg)
        << ",\"record_id\":" << rec.recordId
        << ",\"status\":\"Present\""
        << ",\"is_outside_area\":" << (isOutside ? "true" : "false") << "}";
    return HttpResponse::json(201, oss.str());
}

HttpResponse AttendanceController::editAttendanceRecord(int recordId, const HttpRequest& req, const AuthSession& session) {
    auto body = JsonUtils::parseFlatObject(req.body);
    std::string newStatus = JsonUtils::getString(body, "status");
    std::string reason = JsonUtils::getString(body, "edit_reason");

    if (newStatus != "Present" && newStatus != "Absent" && newStatus != "Late" && newStatus != "Excused") {
        return HttpResponse::error(400, "Invalid status");
    }
    // FR10: Lecturer must enter a reason when editing attendance
    if (reason.empty()) {
        return HttpResponse::error(400, "Edit reason is required and saved in the audit log");
    }

    std::lock_guard<std::mutex> lock(db.mtx);
    AttendanceRecord* rec = db.findRecordById(recordId);
    if (!rec) return HttpResponse::error(404, "Attendance record not found");

    // NFR09: Record who changed it, when, old value, and new value
    rec->oldStatus = rec->status;
    rec->status = newStatus;
    rec->editReason = reason;
    rec->updatedBy = session.userId;
    rec->updatedAt = SecurityUtils::getCurrentDateTimeString();

    std::ostringstream oss;
    oss << "{\"message\":\"Attendance record updated and logged\""
        << ",\"record_id\":" << rec->recordId
        << ",\"old_status\":" << JsonUtils::quote(rec->oldStatus)
        << ",\"new_status\":" << JsonUtils::quote(rec->status)
        << ",\"edit_reason\":" << JsonUtils::quote(rec->editReason)
        << ",\"updated_by\":" << rec->updatedBy
        << ",\"updated_at\":" << JsonUtils::quote(rec->updatedAt) << "}";
    return HttpResponse::json(200, oss.str());
}
