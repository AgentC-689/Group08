#pragma once
#include "models/User.h"
#include <string>

// Part 3 (Member 3): Lecturer profile inheriting from User (FR08)
class Lecturer : public User {
public:
    int lecturerId = 0;
    std::string lecturerCode;
    std::string fullName;
    std::string email;
    std::string department;

    Lecturer() = default;
};
