#include "config.hxx"

#include <fstream>
#include <spdlog/spdlog.h>
#include <ranges>
#include <iostream>
#include <string_view>

void config::loadConfig() {
    std::fstream stream{"cutegirl.conf", stream.in | stream.out};

    if (!stream.is_open())
        return spdlog::error("couldn't load config file'");

    for (std::string line; std::getline(stream, line);) {
        std::string_view setting;
        std::string_view value;
        
        for (auto word : std::views::split(line, ':')) { 
            if (!setting.size()) {
                setting = std::string_view(word);
                continue;
            }

            value = std::string_view(word);
        }

        config::settings[std::string(setting)] = std::string(value);
    }
}
