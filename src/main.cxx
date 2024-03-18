#include <thread>

#include "callback.hxx"
#include <mumlib2.h>
#include "mumlib2/structs.h"

#include "bot.hxx"

using namespace std::chrono_literals;

void runApp() {
    while (true) {
        try {
            mumlib2::Mumlib2 mum(bot::myCallback);
            bot::myCallback.mum = &mum;
            mum.connect(config::settings["server"], std::stoi(config::settings["port"]), config::settings["username"], config::settings["password"]);
            mum.run();
        } catch (mumlib2::TransportException &exp) {
            spdlog::error("TransportException: {}.", exp.what());
            spdlog::info("Attempting to reconnect in 5s.");
            std::this_thread::sleep_for(5s);
        }
    }
}

int main(int argc, char *argv[]) {
    config::loadConfig();
    bot::init();
    
    std::thread libraryThread{runApp};
    std::thread statsThread{bot::requestStats};

    libraryThread.join();
    statsThread.join();
}
