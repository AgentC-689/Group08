#pragma once
#include "models/User.h"
#include <string>

// Part 2 (Member 2): Student profile inheriting from User (FR04, FR05)
class Student : public User {
public:
    int studentId = 0;
    std::string studentCode;
    std::string fullName;
    std::string dateOfBirth;
    std::string email;
    std::string phoneNumber;
    int classId = 0;

    Student() = default;
};
