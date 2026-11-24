#include "controllers/ReportController.h"
#include "config/Config.h"
#include "core/JsonUtils.h"
#include <iomanip>
#include <sstream>

ReportController::ReportController(DatabaseContext& database) : db(database) {}

HttpResponse ReportController::getDashboardSummary() {
    std::lock_guard<std::mutex> lock(db.mtx);
    int totalRecords = static_cast<int>(db.records.size());
    int attended = 0;
    for (const auto& r : db.records) {
        if (r.status == "Present" || r.status == "Late") ++attended;
    }
    double rate = totalRecords > 0 ? (100.0 * attended / totalRecords) : 100.0;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1)
        << "{\"total_students\":" << db.students.size()
        << ",\"total_classes\":" << db.classes.size()
        << ",\"total_courses\":" << db.courses.size()
        << ",\"attendance_rate\":" << rate << "}";
    return HttpResponse::json(200, oss.str());
}

HttpResponse ReportController::getAttendanceHistory(const HttpRequest& req, const AuthSession& session) {
    std::string filterStatus = req.queryParams.count("status") ? req.queryParams.at("status") : "";
    std::string filterDate = req.queryParams.count("date") ? req.queryParams.at("date") : "";

    std::lock_guard<std::mutex> lock(db.mtx);
    int ownStudentId = 0;
    int ownLecturerId = 0;
    if (session.role == "Student") {
        Student* s = db.findStudentByUserId(session.userId);
        if (s) ownStudentId = s->studentId;
    } else if (session.role == "Lecturer") {
        Lecturer* l = db.findLecturerByUserId(session.userId);
        if (l) ownLecturerId = l->lecturerId;
    }

    std::vector<std::string> items;
    for (const auto& r : db.records) {
        AttendanceSession* sess = db.findSessionById(r.sessionId);
        Student* st = db.findStudentById(r.studentId);
        if (!sess || !st) continue;

        // NFR04: Enforce strict data scoping per role
        if (session.role == "Student" && r.studentId != ownStudentId) continue;
        if (session.role == "Lecturer" && sess->lecturerId != ownLecturerId) continue;
        if (!filterStatus.empty() && r.status != filterStatus) continue;
        if (!filterDate.empty() && sess->sessionDate != filterDate) continue;

        ClassModel* cls = db.findClassById(sess->classId);
        std::ostringstream oss;
        oss << "{\"record_id\":" << r.recordId
            << ",\"student_code\":" << JsonUtils::quote(st->studentCode)
            << ",\"student_name\":" << JsonUtils::quote(st->fullName)
            << ",\"class_code\":" << JsonUtils::quote(cls ? cls->classCode : "")
            << ",\"session_date\":" << JsonUtils::quote(sess->sessionDate)
            << ",\"status\":" << JsonUtils::quote(r.status)
            << ",\"old_status\":" << JsonUtils::quote(r.oldStatus)
            << ",\"is_outside_area\":" << (r.isOutsideArea ? "true" : "false")
            << ",\"edit_reason\":" << JsonUtils::quote(r.editReason) << "}";
        items.push_back(oss.str());
    }
    return HttpResponse::json(200, JsonUtils::joinArray(items));
}

HttpResponse ReportController::getAttendanceReport(const HttpRequest& req, const AuthSession& session) {
    int filterClassId = 0;
    if (req.queryParams.count("class_id") && !req.queryParams.at("class_id").empty()) {
        try { filterClassId = std::stoi(req.queryParams.at("class_id")); } catch (...) {}
    }

    std::lock_guard<std::mutex> lock(db.mtx);
    int ownStudentId = 0;
    if (session.role == "Student") {
        Student* st = db.findStudentByUserId(session.userId);
        if (st) ownStudentId = st->studentId;
    }

    std::vector<std::string> rows;
    int sumPresent = 0, sumAbsent = 0, sumLate = 0, sumExcused = 0;

    for (const auto& st : db.students) {
        if (session.role == "Student" && st.studentId != ownStudentId) continue;
        if (filterClassId > 0 && st.classId != filterClassId) continue;

        int total = 0, present = 0, absent = 0, late = 0, excused = 0;
        for (const auto& r : db.records) {
            if (r.studentId != st.studentId) continue;
            ++total;
            if (r.status == "Present") ++present;
            else if (r.status == "Absent") ++absent;
            else if (r.status == "Late") ++late;
            else if (r.status == "Excused") ++excused;
        }
        sumPresent += present;
        sumAbsent += absent;
        sumLate += late;
        sumExcused += excused;

        double absenceRate = total > 0 ? (100.0 * absent / total) : 0.0;
        double attendanceRate = total > 0 ? (100.0 * (present + late) / total) : 100.0;
        // FR14: Flag students whose absence rate exceeds 20%
        bool warning = absenceRate > Config::DEFAULT_ABSENCE_WARNING_PERCENT;

        std::ostringstream row;
        row << std::fixed << std::setprecision(1)
            << "{\"student_id\":" << st.studentId
            << ",\"student_code\":" << JsonUtils::quote(st.studentCode)
            << ",\"full_name\":" << JsonUtils::quote(st.fullName)
            << ",\"total_sessions\":" << total
            << ",\"present\":" << present
            << ",\"absent\":" << absent
            << ",\"late\":" << late
            << ",\"excused\":" << excused
            << ",\"attendance_rate\":" << attendanceRate
            << ",\"absence_rate\":" << absenceRate
            << ",\"absence_warning\":" << (warning ? "true" : "false") << "}";
        rows.push_back(row.str());
    }

    int grandTotal = sumPresent + sumAbsent + sumLate + sumExcused;
    double overallPresentPct = grandTotal > 0 ? (100.0 * (sumPresent + sumLate) / grandTotal) : 100.0;
    double overallAbsentPct = grandTotal > 0 ? (100.0 * sumAbsent / grandTotal) : 0.0;

    std::ostringstream out;
    out << std::fixed << std::setprecision(1)
        << "{\"overall_present_percent\":" << overallPresentPct
        << ",\"overall_absent_percent\":" << overallAbsentPct
        << ",\"students\":" << JsonUtils::joinArray(rows) << "}";
    return HttpResponse::json(200, out.str());
}

HttpResponse ReportController::exportReportCsv(const HttpRequest& req, const AuthSession& session) {
    (void)req;
    (void)session;
    std::lock_guard<std::mutex> lock(db.mtx);
    std::ostringstream csv;
    csv << "StudentCode,FullName,TotalSessions,Present,Absent,Late,Excused,AbsenceRate(%),Warning(>20%)\n";
    for (const auto& st : db.students) {
        int total = 0, present = 0, absent = 0, late = 0, excused = 0;
        for (const auto& r : db.records) {
            if (r.studentId != st.studentId) continue;
            ++total;
            if (r.status == "Present") ++present;
            else if (r.status == "Absent") ++absent;
            else if (r.status == "Late") ++late;
            else if (r.status == "Excused") ++excused;
        }
        double absenceRate = total > 0 ? (100.0 * absent / total) : 0.0;
        csv << st.studentCode << "," << st.fullName << "," << total << ","
            << present << "," << absent << "," << late << "," << excused << ","
            << std::fixed << std::setprecision(1) << absenceRate << ","
            << (absenceRate > Config::DEFAULT_ABSENCE_WARNING_PERCENT ? "YES" : "NO") << "\n";
    }
    HttpResponse res;
    res.statusCode = 200;
    res.contentType = "text/csv; charset=utf-8";
    res.extraHeaders["Content-Disposition"] = "attachment; filename=\"attendance_report.csv\"";
    res.body = csv.str();
    return res;
}
