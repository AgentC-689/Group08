#pragma once
#include <string>

// Global application configuration constants
namespace Config {
    inline constexpr int SERVER_PORT = 8080;
    // NFR03: Automatic session timeout after 30 minutes of inactivity
    inline constexpr int SESSION_TIMEOUT_SECONDS = 30 * 60;
    // FR15: Default allowed GPS radius in meters
    inline constexpr double DEFAULT_ALLOWED_DISTANCE_METERS = 100.0;
    // FR14: Default institutional absence warning threshold (20%)
    inline constexpr double DEFAULT_ABSENCE_WARNING_PERCENT = 20.0;
    // FR10: Allowed post-session edit grace window in hours
    inline constexpr int EDIT_WINDOW_HOURS = 24;
    // Static frontend directory path relative to project root
    inline const std::string FRONTEND_DIR = "Frontend";
}
