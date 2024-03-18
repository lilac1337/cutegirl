#pragma once

#include <cstdint>
#include <mumlib2.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <SQLiteCpp/SQLiteCpp.h>

#include "callback.hxx"

struct Command;

struct mumbleUser {
    std::int32_t key{};
    std::string name{};
    std::uint32_t sessionId{};
    std::uint32_t connectedTime{};
    std::uint32_t talkingTime{};
    std::int32_t channel{};
};

namespace bot {
    inline std::uint32_t self{};
    inline std::int32_t currentChannel = -1;
    inline std::vector<mumbleUser> users;
    inline std::vector<Command> commands;
    inline callback myCallback;

    inline SQLite::Database db("db.sq3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

    void addCommand(const Command& command);

    void sendLeaderboard(std::uint32_t actor, bool pm);
    void bring(std::uint32_t actor, bool pm);
    
    void handleDisconnect(std::uint32_t sessionId);
    void handleConnect(std::uint32_t sessionId, std::int32_t channelId);
    void handleStats(std::uint32_t sessionId, std::uint32_t onlineSecs, std::uint32_t idleSecs);

    void requestStats();

    void init();
    
    void sqlInit();
    void sqlUpdate(mumbleUser* user, std::uint32_t addedTime, bool talking = false, std::uint32_t idleTime = 0u);
};

struct Command {
	std::string name;
    std::function<void(std::uint32_t, bool pm)> func;
};
