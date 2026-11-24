#pragma once
#include <string>

// Part 3 (Member 3): Academic course entity (FR06)
class Course {
public:
    int courseId = 0;
    std::string courseCode;
    std::string courseName;
    int credits = 3;

    Course() = default;
};
