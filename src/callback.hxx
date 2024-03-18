#pragma once

#include <mumlib2.h>
#include <spdlog/spdlog.h>

#include "config.hxx"
#include "mumlib2/callback.h"
#include "mumlib2/structs.h"

class callback : public mumlib2::Callback {
public:
	std::atomic<mumlib2::Mumlib2*> mum;

    virtual void textMessage(uint32_t actor, std::vector<uint32_t> session, std::vector<uint32_t> channel_id, std::vector<uint32_t> tree_id, std::string message) override;
	virtual void userRemove(uint32_t session,int32_t actor,std::string reason,bool ban) override;
    virtual void userState(int32_t session,int32_t actor, std::string name, int32_t user_id, int32_t channel_id, int32_t mute, int32_t deaf, int32_t suppress, int32_t self_mute, int32_t self_deaf, std::string comment, int32_t priority_speaker, int32_t recording) override;
	virtual void userStats(uint32_t sessionId, uint32_t onlinesecs, uint32_t idlesecs) override;
};
