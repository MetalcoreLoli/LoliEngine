#include <iostream>
#include "../src/LoliEngine.hpp"

struct MyConsoleLogger : public loli::utils::ILogger {
    void log (const std::string& msg) override {
        std::cout << "[" << __BASE_FILE__ << ":" << __LINE__ << "]: " << msg << std::endl;
    }
};

struct MyApp : public loli::LoliApp {
    explicit MyApp(loli::utils::ILogger *logger) : loli::LoliApp(logger) {}

    loli::LoliApp& OnKeyDown (SDL_Keycode key) override {
        switch (key) {
            case SDLK_ESCAPE:
                changeStateTo(loli::AppState::QUIT);
                break;
        }
    }
};

int main() {
    MyApp app(new loli::utils::ConsoleLogger);
    auto subscription = app.KeyDownEvent.subscribe(&app)->add([](auto& s, auto& key) {
        std::cout << "key: " << (char) key.code.get() << " was pressed" <<std::endl;
    });


    app.screenWidth(600).screenHeight(800).run();
    return 0;
}
