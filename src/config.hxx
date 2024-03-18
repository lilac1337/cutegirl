#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <variant>

namespace config {
//	inline char prefix = ';';

    inline std::unordered_map<std::string, std::string> settings;

/*	inline std::string server = "cutehops.net";
    inline std::uint16_t port = 64738;
    inline std::string username = "cutegirl";
    inline std::string password = "superSecurePassword";*/

    void loadConfig();
};
