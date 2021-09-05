#include <iostream>
#include "../src/LoliEngine.hpp"

struct MyConsoleLogger : public loli::utils::ILogger {
    void log (const std::string& msg) override {
        std::cout << "[" << __BASE_FILE__ << ":" << __LINE__ << "]: " << msg << std::endl;
    }
};

struct MyApp : public loli::LoliApp {
    explicit MyApp(loli::utils::ILogger *logger) : loli::LoliApp(logger) {
        auto *sub = KeyDownEvent.subscribe(this)->add([&](auto &key) mutable {
            loli::utils::ConsoleLogger log;
            std::string msg = "key: ";
            log.log(msg.append(std::to_string((char)key->code.get()).append(" was pressed")));
        });
        KeyDownEvent.remove(sub);
    }

    loli::LoliApp& OnKeyDown (SDL_Keycode key) override {
        switch (key) {
            case SDLK_q:
                changeStateTo(loli::AppState::QUIT);
                break;
        }
    }
};

int main() {
    MyApp app(new loli::utils::ConsoleLogger);
    app.screenWidth(600).screenHeight(800).run();
    return 0;
}
