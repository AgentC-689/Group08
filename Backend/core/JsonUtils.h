#pragma once
#include <map>
#include <string>
#include <vector>

// Zero-dependency JSON helper for serializing and parsing flat JSON objects
namespace JsonUtils {
    std::string escape(const std::string& input);
    std::string quote(const std::string& input);
    std::map<std::string, std::string> parseFlatObject(const std::string& jsonText);
    std::string getString(const std::map<std::string, std::string>& obj, const std::string& key, const std::string& fallback = "");
    int getInt(const std::map<std::string, std::string>& obj, const std::string& key, int fallback = 0);
    double getDouble(const std::map<std::string, std::string>& obj, const std::string& key, double fallback = 0.0);
    bool getBool(const std::map<std::string, std::string>& obj, const std::string& key, bool fallback = false);
    std::string joinArray(const std::vector<std::string>& items);
}
