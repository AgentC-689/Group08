#pragma once
#include <string>

// Cryptographic hashing, session token, and dynamic code utilities (NFR03, FR15)
namespace SecurityUtils {
    std::string hashPassword(const std::string& plainPassword);
    bool verifyPassword(const std::string& plainPassword, const std::string& storedHash);
    std::string generateRandomToken(size_t length = 32);
    std::string generateSessionCode(size_t length = 6);
    std::string getCurrentDateTimeString();
    std::string getCurrentDateString();
    std::string getCurrentTimeString();
}
