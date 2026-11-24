#include "core/SecurityUtils.h"
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>
#include <cstdint>

namespace SecurityUtils {

// Deterministic salted FNV-1a + mixing hash (replace with BCrypt/Argon2 in production)
std::string hashPassword(const std::string& plainPassword) {
    const std::string salted = "SAS_SALT_2026::" + plainPassword + "::END";
    uint64_t h1 = 14695981039346656037ULL;
    uint64_t h2 = 1099511628211ULL;
    for (unsigned char c : salted) {
        h1 ^= c;
        h1 *= 1099511628211ULL;
        h2 = (h2 * 1315423911ULL) ^ ((h2 << 5) + c + (h2 >> 2));
    }
    std::ostringstream oss;
    oss << "hash$" << std::hex << std::setfill('0')
        << std::setw(16) << h1 << std::setw(16) << h2;
    return oss.str();
}

bool verifyPassword(const std::string& plainPassword, const std::string& storedHash) {
    return hashPassword(plainPassword) == storedHash;
}

std::string generateRandomToken(size_t length) {
    static const char chars[] = "0123456789abcdef";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, sizeof(chars) - 2);
    std::string token;
    token.reserve(length);
    for (size_t i = 0; i < length; ++i) token += chars[dist(gen)];
    return token;
}

std::string generateSessionCode(size_t length) {
    static const char chars[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, sizeof(chars) - 2);
    std::string code;
    code.reserve(length);
    for (size_t i = 0; i < length; ++i) code += chars[dist(gen)];
    return code;
}

std::string getCurrentDateTimeString() {
    std::time_t now = std::time(nullptr);
    std::tm tmBuf{};
#ifdef _WIN32
    localtime_s(&tmBuf, &now);
#else
    localtime_r(&now, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string getCurrentDateString() {
    return getCurrentDateTimeString().substr(0, 10);
}

std::string getCurrentTimeString() {
    return getCurrentDateTimeString().substr(11, 5); // HH:MM
}

} // namespace SecurityUtils
