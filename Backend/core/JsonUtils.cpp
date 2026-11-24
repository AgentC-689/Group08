#include "core/JsonUtils.h"
#include <cctype>
#include <sstream>

namespace JsonUtils {

std::string escape(const std::string& input) {
    std::string out;
    out.reserve(input.size() + 8);
    for (char c : input) {
        switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

std::string quote(const std::string& input) {
    return "\"" + escape(input) + "\"";
}

// Parses a flat JSON object {"key":"value", "num":123, "flag":true} into a string map
std::map<std::string, std::string> parseFlatObject(const std::string& jsonText) {
    std::map<std::string, std::string> result;
    size_t i = 0;
    const size_t n = jsonText.size();

    auto skipWs = [&]() {
        while (i < n && std::isspace(static_cast<unsigned char>(jsonText[i]))) ++i;
    };

    auto parseQuoted = [&]() -> std::string {
        std::string val;
        if (i < n && jsonText[i] == '\"') ++i;
        while (i < n && jsonText[i] != '\"') {
            if (jsonText[i] == '\\' && i + 1 < n) {
                ++i;
                if (jsonText[i] == 'n') val += '\n';
                else if (jsonText[i] == 'r') val += '\r';
                else if (jsonText[i] == 't') val += '\t';
                else val += jsonText[i];
            } else {
                val += jsonText[i];
            }
            ++i;
        }
        if (i < n && jsonText[i] == '\"') ++i;
        return val;
    };

    skipWs();
    if (i < n && jsonText[i] == '{') ++i;

    while (i < n) {
        skipWs();
        if (i >= n || jsonText[i] == '}') break;
        if (jsonText[i] == ',') { ++i; continue; }
        if (jsonText[i] != '\"') { ++i; continue; }

        std::string key = parseQuoted();
        skipWs();
        if (i < n && jsonText[i] == ':') ++i;
        skipWs();

        std::string value;
        if (i < n && jsonText[i] == '\"') {
            value = parseQuoted();
        } else {
            size_t start = i;
            while (i < n && jsonText[i] != ',' && jsonText[i] != '}' && !std::isspace(static_cast<unsigned char>(jsonText[i]))) {
                ++i;
            }
            value = jsonText.substr(start, i - start);
        }
        result[key] = value;
    }
    return result;
}

std::string getString(const std::map<std::string, std::string>& obj, const std::string& key, const std::string& fallback) {
    auto it = obj.find(key);
    return (it != obj.end() && it->second != "null") ? it->second : fallback;
}

int getInt(const std::map<std::string, std::string>& obj, const std::string& key, int fallback) {
    auto it = obj.find(key);
    if (it == obj.end() || it->second.empty() || it->second == "null") return fallback;
    try { return std::stoi(it->second); } catch (...) { return fallback; }
}

double getDouble(const std::map<std::string, std::string>& obj, const std::string& key, double fallback) {
    auto it = obj.find(key);
    if (it == obj.end() || it->second.empty() || it->second == "null") return fallback;
    try { return std::stod(it->second); } catch (...) { return fallback; }
}

bool getBool(const std::map<std::string, std::string>& obj, const std::string& key, bool fallback) {
    auto it = obj.find(key);
    if (it == obj.end()) return fallback;
    if (it->second == "true" || it->second == "1") return true;
    if (it->second == "false" || it->second == "0") return false;
    return fallback;
}

std::string joinArray(const std::vector<std::string>& items) {
    std::ostringstream oss;
    oss << "[";
    for (size_t idx = 0; idx < items.size(); ++idx) {
        if (idx > 0) oss << ",";
        oss << items[idx];
    }
    oss << "]";
    return oss.str();
}

} // namespace JsonUtils
