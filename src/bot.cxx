#include "bot.hxx"
#include "config.hxx"
#include <chrono>
#include <exception>
#include <ranges>

using namespace std::chrono_literals;

void bot::addCommand(const Command& command) {
    commands.emplace_back(command);
}

void bot::sendLeaderboard(std::uint32_t actor, bool pm) {
    if (!users.size())
        return;
    
    auto mum = bot::myCallback.mum.load();
    
    std::vector<std::tuple<std::string, std::uint32_t, std::uint32_t>> rankings;

    for (auto&& user : users)
        rankings.emplace_back(user.name, user.connectedTime, user.talkingTime);

    std::ranges::sort(rankings, std::greater<>{}, [](auto&& t) { return std::get<1>(t); });
    std::string message = "<br>~~ Connection Leaderboard ~~<br>";

    for (size_t i{}; i < rankings.size(); ++i) {
        auto& [name, onlineSecs, talkingSecs] = rankings[i];

        if (!onlineSecs)
            continue;
        
        std::chrono::seconds seconds(onlineSecs);
        
        message.append(std::format("#{} ~  {}: {:%Hh%Mm%Ss}<br>", i + 1, name, seconds));
    }

    std::ranges::sort(rankings, std::greater<>{}, [](auto&& t) { return std::get<2>(t); });
    message.append("<br>~~ Speaking Leaderboard ~~<br>");

    for (size_t i{}; i < rankings.size(); ++i) {
        auto& [name, onlineSecs, talkingSecs] = rankings[i];

        if (!talkingSecs)
            continue;
        
        std::chrono::seconds talking(talkingSecs);
        
        message.append(std::format("#{} ~  {}: {:%Hh%Mm%Ss}<br>", i + 1, name, talking));
    }
    
    mum->sendTextMessage(message);
}

void bot::bring(std::uint32_t actor, bool pm) {
    if (!pm)
        return;

    auto found = std::ranges::find(users, actor, &mumbleUser::sessionId);
        
    if (found == users.end())
        return;

    auto mum = bot::myCallback.mum.load();

    mum->ChannelJoin(found->channel);
}

void bot::handleDisconnect(std::uint32_t sessionId) {
    if (sessionId == self)
        return;
    
    auto found = std::ranges::find(users, sessionId, &mumbleUser::sessionId);
        
    if (found == users.end())
        return;

    spdlog::info("{} has disconnected with {} total seconds", found->name, found->connectedTime);

     found->sessionId = 0u;
}

void bot::handleConnect(std::uint32_t sessionId, std::int32_t channelId) {
    if (sessionId == self)
        currentChannel = channelId;

    auto found = std::ranges::find(users, bot::myCallback.mum.load()->UserGet(sessionId)->name, &mumbleUser::name);

    if (found == users.end())
        return;

    spdlog::info("{} has connected/changed with a total of {} onlinsecs", found->name, found->connectedTime);

    found->sessionId = sessionId;
    found->channel = channelId;
}

void bot::handleStats(std::uint32_t sessionId, std::uint32_t onlineSecs, std::uint32_t idleSecs) {
    if (sessionId == self)
        return;
    
    auto found = std::ranges::find(users, bot::myCallback.mum.load()->UserGet(sessionId)->name, &mumbleUser::name);

    if (found != users.end()) {
        sqlUpdate(found.base(), 5u, (idleSecs <= 5u), idleSecs);

        found->sessionId = sessionId;
        
        return;
    }

    mumbleUser newUser;
    
    newUser.name =  bot::myCallback.mum.load()->UserGet(sessionId)->name;
    newUser.sessionId = sessionId;
    newUser.connectedTime = onlineSecs;
    newUser.key = users.size();
    
    try {
		SQLite::Transaction transaction(db);

		db.exec(std::format("INSERT INTO USERS (ID, NAME, ONLINESECS) VALUES({}, \'{}\', {});", users.size(), newUser.name.c_str(), onlineSecs));

		transaction.commit();
	}
	catch (std::exception& e) {
	    spdlog::info("bot::handleStats sql exception: {}", e.what());
	}

    users.push_back(newUser);
    
    spdlog::info("inserted user \"{}\" onlineSecs = {}", newUser.name, onlineSecs);
}

void bot::requestStats() {
    while ("swag") {
        auto mum = myCallback.mum.load();

        if (!mum) {
            spdlog::info("mumlib not yet loaded");
        } else {
            std::vector<mumlib2::MumbleUser> users = mum->getListAllUser();

            for (auto &&user : users) {
                if (user.name == config::settings["username"]) {
                     self = user.sessionId;
                     continue;
                }
                
                mum->requestUserStats(user.sessionId, true);
            }
            
            std::this_thread::sleep_for(5s);
        }
    }
}

void bot::init() {
    addCommand({"lb", sendLeaderboard});
    addCommand({"leaderboard", sendLeaderboard});
    addCommand({"bring", bring});
    
    sqlInit();
}

void bot::sqlInit() {
	try
	{
		SQLite::Transaction transaction(db);

		db.exec("CREATE TABLE IF NOT EXISTS users(ID INT PRIMARY KEY NOT NULL, NAME CHARACTER(255) NOT NULL, ONLINESECS UNSIGNED INT, TALKINGSECS UNSIGNED INT)");

		transaction.commit();

		SQLite::Statement query(db, "SELECT * FROM users");

		while (query.executeStep()) {
			const std::int32_t key = query.getColumn(0);
            const std::string name = query.getColumn(1);
            const std::uint32_t onlineSecs = query.getColumn(2);
            const std::uint32_t talkingSecs = query.getColumn(3);

            spdlog::info("pushed {} with {} onlineSecs", name, onlineSecs);
            
			users.push_back({key, name, 0u, onlineSecs, talkingSecs});
		}
	}
	catch (std::exception& e) {
        spdlog::info("sqlInit exception: {}", e.what());
	}
}

void bot::sqlUpdate(mumbleUser* user, std::uint32_t addedTime, bool talking, std::uint32_t idleTime) {
    user->connectedTime += addedTime;

    try {
            SQLite::Transaction transaction(db);

            if (talking && (currentChannel == user->channel)) {
                user->talkingTime += addedTime - idleTime;
                
                db.exec(std::format("UPDATE users SET ONLINESECS = {}, TALKINGSECS = {} WHERE ID = {}", user->connectedTime, user->talkingTime, user->key));
            } else {
                db.exec(std::format("UPDATE users SET ONLINESECS = {} WHERE ID = {}", user->connectedTime, user->key));
            }
            
            transaction.commit();
    } catch (std::exception& e) {
        spdlog::info("bot::sqlUpdate exception: {}", e.what());
    }
}
