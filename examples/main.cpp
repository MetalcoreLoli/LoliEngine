#include <iostream>
#include <GLFW/glfw3.h>
#include "../src/LoliEngine.hpp"

class SDLWindow : public loli::graphics::Window {
public:
    SDLWindow(){}
    SDLWindow (const std::string& name, loli::utils::ILogger* logger = new loli::utils::ConsoleLogger) {
        this->name(name);
        _mLogger = logger;
    }
    virtual ~SDLWindow() {destroy();}
private:
    SDL_Window *_mWindow = nullptr;
    loli::utils::ILogger* _mLogger;
protected:
    void init() override {
        SDL_Init(SDL_INIT_EVERYTHING);
        _mWindow = SDL_CreateWindow(
                name().c_str(),
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                screenHeight(), screenWidth(),
                SDL_WINDOW_OPENGL);
        if (_mWindow == nullptr) {
            _mLogger->log("SDL window was not created");
            SDL_Quit();
        }

        SDL_GLContext context = SDL_GL_CreateContext(_mWindow);
        if (context == nullptr) {
            _mLogger->log("SDL window was not created");
            SDL_Quit();
        }
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }

    void processInput () override {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    loli::events::args::EmptyEventArgs emptyEventArgs;
                    OnClosingEvent.invoke(emptyEventArgs);
                    break;
                case SDL_KEYDOWN:
                    loli::events::args::KeyDownEventArgs eventArgs(event.key.keysym.sym);
                    KeyDownEvent.invoke(eventArgs);
                    break;
            }
        }
    }

    void draw() override {
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: create OnDrawEventArgs und pass it here
        loli::events::args::EmptyEventArgs eventArgs;
        DrawEvent.invoke(eventArgs);
        SDL_GL_SwapWindow(_mWindow);
    }

    void destroy () override {
        SDL_DestroyWindow(_mWindow);
    }
};

struct MyApp : public loli::LoliApp {
    MyApp(loli::graphics::Window* win, loli::utils::ILogger *logger) : loli::LoliApp(win, logger) {
        KeyDownEvent.subscribe(this)->add([&](auto s, auto e) {
            OnKeyDown(e.code.get());
            std::cout << (char)e.code.get() << std::endl;
        });
    }

    loli::LoliApp& OnKeyDown (SDL_Keycode key) override {
        switch (key) {
            case SDLK_ESCAPE:
                changeStateTo(loli::AppState::QUIT);
                break;
        }
    }
};

int main() {
    loli::IAppConfiguration *configuration = new loli::DefAppConfiguration<MyApp>;
    loli::graphics::DefWinConfiguration<SDLWindow> winConfiguration;

    winConfiguration.screenWidth(600).screenHeight(800);
    configuration->logger(new loli::utils::ConsoleLogger).window(winConfiguration);

    //auto app = loli::LoliApp::FromConfiguration<MyApp>(*configuration);
    SDLWindow *win = new SDLWindow("Loli Engine", new loli::utils::ConsoleLogger);
    win->screenHeight(800);
    win->screenWidth(600);

    MyApp app (win, new loli::utils::ConsoleLogger);

    app.KeyDownEvent.subscribe(&app)->add([](auto& s, auto& key) mutable {
        std::cout << "key: " << (char) key.code.get() << " was pressed" <<std::endl;
    });
    app.run();
    return 0;
}


void foo(){
    //GLFWwindow* window;

    ///* Initialize the library */
    //if (!glfwInit())
    //    return -1;

    ///* Create a windowed mode window and its OpenGL context */
    //window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    //if (!window)
    //{
    //    glfwTerminate();
    //    return -1;
    //}

    ///* Make the window's context current */
    //glfwMakeContextCurrent(window);

    ///* Loop until the user closes the window */
    //while (!glfwWindowShouldClose(window))
    //{
    //    /* Render here */
    //    glClear(GL_COLOR_BUFFER_BIT);

    //    /* Swap front and back buffers */
    //    glfwSwapBuffers(window);

    //    /* Poll for and process events */
    //    glfwPollEvents();
    //}

    //glfwTerminate();*/
}