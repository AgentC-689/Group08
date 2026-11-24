#pragma once
#include <string>

// Part 4 (Member 4): Per-student attendance record with GPS flag & audit trail (FR09, FR10, FR16, NFR09)
class AttendanceRecord {
public:
    int recordId = 0;
    int sessionId = 0;
    int studentId = 0;
    std::string oldStatus;       // NFR09: Previous status before edit
    std::string status;          // "Present", "Absent", "Late", "Excused"
    std::string checkInTime;
    double deviceLatitude = 0.0;
    double deviceLongitude = 0.0;
    bool hasLocation = false;
    bool isOutsideArea = false;  // FR16: Flagged if check-in exceeds allowed GPS distance
    std::string editReason;      // FR10: Mandatory reason when edited by lecturer
    int updatedBy = 0;           // NFR09: User ID who made the change
    std::string updatedAt;       // NFR09: Timestamp of the change
};
