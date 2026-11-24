#pragma once
#include <string>

// Part 2 (Member 2): Class cohort entity with lecturer, room, and schedule (FR07)
class ClassModel {
public:
    int classId = 0;
    std::string classCode;
    std::string className;
    std::string academicYear;
    int courseId = 0;
    int lecturerId = 0;
    std::string schedule;
    std::string room;

    ClassModel() = default;
};
