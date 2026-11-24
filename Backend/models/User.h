#pragma once
#include <string>

// Part 1 (Member 1): Base class for system authentication and RBAC (FR01-FR03)
class User {
public:
    int userId = 0;
    std::string username;
    std::string passwordHash;
    std::string role;       // "Administrator", "Lecturer", "Student"
    bool isLocked = false;
    std::string createdAt;

    User() = default;
    User(int id, std::string uname, std::string pwdHash, std::string r, bool locked, std::string created)
        : userId(id), username(std::move(uname)), passwordHash(std::move(pwdHash)),
          role(std::move(r)), isLocked(locked), createdAt(std::move(created)) {}
};
