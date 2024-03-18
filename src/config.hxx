#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <variant>

namespace config {
    inline std::unordered_map<std::string, std::string> settings;
    
    void loadConfig();
};
