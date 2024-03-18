#include "callback.hxx"
#include <iostream>
#include "bot.hxx"
#include "config.hxx"
#include "mumlib2/callback.h"

void callback::textMessage(uint32_t actor, std::vector<uint32_t> session, std::vector<uint32_t> channel_id, std::vector<uint32_t> tree_id, std::string message) {
    mumlib2::Callback::textMessage(actor, session, channel_id, tree_id, message);
  
    if (message[0] == config::settings["prefix"][0]) {
        auto found = std::ranges::find(bot::commands, message.substr(1), &Command::name);
        
        if (found == bot::commands.end())
            return;

        found->func(actor, !(channel_id.size()));
        
        spdlog::info("recieved command: {}", message);
    }
}

void callback::userRemove(uint32_t session, int32_t actor, std::string reason, bool ban)  {
    mumlib2::Callback::userRemove(session, actor, reason, ban);

    bot::handleDisconnect(session);
};

void callback::userState(int32_t session,int32_t actor, std::string name, int32_t user_id, int32_t channel_id, int32_t mute, int32_t deaf, int32_t suppress, int32_t self_mute, int32_t self_deaf, std::string comment, int32_t priority_speaker, int32_t recording) {
    mumlib2::Callback::userState(session, actor, name, user_id, channel_id, mute, deaf, suppress, self_mute, self_deaf, comment, priority_speaker, recording);

    bot::handleConnect(session, channel_id);
}

void callback::userStats(uint32_t sessionId, uint32_t onlinesecs, uint32_t idlesecs) {
    mumlib2::Callback::userStats(sessionId, onlinesecs, idlesecs);

    bot::handleStats(sessionId, onlinesecs, idlesecs);
  
    //spdlog::info("userStats: sessionId ({}), onlinesecs ({})", sessionId,onlinesecs);
}
