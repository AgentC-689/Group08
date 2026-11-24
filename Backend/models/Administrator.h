#pragma once
#include "models/User.h"
#include <string>

// Part 1 (Member 1): Administrator profile inheriting from User
class Administrator : public User {
public:
    int adminId = 0;
    std::string adminCode;
    std::string fullName;
    std::string email;

    Administrator() = default;
};
