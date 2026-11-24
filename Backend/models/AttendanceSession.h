#pragma once
#include <string>

// Part 4 (Member 4): Attendance session with time window, code & GPS policy (FR15, UC08)
class AttendanceSession {
public:
    int sessionId = 0;
    int classId = 0;
    int lecturerId = 0;
    std::string sessionDate;     // YYYY-MM-DD
    std::string timeSlot;        // e.g., "Slot 1 (07:00-09:30)"
    std::string startTime;       // HH:MM
    std::string endTime;         // HH:MM
    std::string sessionCode;
    bool requireLocation = false;
    double classLatitude = 0.0;
    double classLongitude = 0.0;
    double allowedDistance = 100.0;
    std::string sessionStatus = "Open"; // "Open" or "Closed"

    // Validate time window constraint (UC08 Alternative Flow 3a)
    bool hasValidTimeRange() const {
        return !startTime.empty() && !endTime.empty() && endTime > startTime;
    }

    // Check if check-in is allowed at the given date and time (FR15, FR16)
    bool isOpenAt(const std::string& currentDate, const std::string& currentTime) const {
        return sessionStatus == "Open" &&
               sessionDate == currentDate &&
               currentTime >= startTime &&
               currentTime <= endTime;
    }
};
