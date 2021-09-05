#include <iostream>
#include "src/LoliEngine.hpp"

int main() {
    loli::LoliApp app(new loli::utils::ConsoleLogger);
    app.screenWidth(600).screenHeight(800).run();
    return 0;
}
