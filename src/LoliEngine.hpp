//
// Created by danielf on 05.09.2021.
//

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GLES3/gl3.h>

#ifndef LOLIENGINE_LOLIENGINE_HPP
#define LOLIENGINE_LOLIENGINE_HPP

namespace loli {
    namespace utils {
        struct ILogger {
            virtual void log (const std::string& msg) = 0;
        };

        struct ConsoleLogger : public ILogger {
            void log(const std::string& msg) override {
                std::cout << "[INFO]: " << msg << std::endl;
            }
        };
        template<typename T>
        struct Property {
            explicit Property(T &value) : _value(value){}
            virtual T get () const {
                return _value;
            }

            virtual void set (T value) {
                if (_value == value)
                    return;
                _value = value;
            }

        private:
            T _value;
        };
    }

    namespace events {
        typedef void eventAction
        struct IEventArgs;

        template<typename T>
        concept EventArguments = std::derived_from<IEventArgs, T>;

        template<typename TSender, EventArguments TEventArgs>
        struct Event {
            struct Subcription;

            Subcription* subcribe() {
                return nullptr;
            }

            void invoke (TSender sender, TEventArgs eventArgs);

        private:
            std::vector<Subcription*> _vSubscriptions{};
        };
    }

    namespace graphics {
        class Sprite {
        public:
            ~Sprite() {
                if (_vboId != 0) {
                    glDeleteBuffers(1, &_vboId);
                }
            }

            void init  (float x, float y, float w, float h) {
                mX.set(x);
                mY.set(y);
                mWidth.set(w);
                mHeight.set(h);
                if (_vboId == 0) {
                    glGenBuffers(1, &_vboId);
                }

                float vertexData [12];
                vertexData[0] = mX.get() + mWidth.get();
                vertexData[1] = mY.get() + mWidth.get();

                vertexData[2] = mX.get();
                vertexData[3] = mY.get() + mHeight.get();

                vertexData[4] = mX.get();
                vertexData[5] = mY.get();

                // second triangle

                vertexData[6] = mX.get();
                vertexData[7] = mY.get();

                vertexData[8] = mX.get() + mWidth.get();
                vertexData[9] = mY.get() ;

                vertexData[10] = mX.get() + mHeight.get();
                vertexData[11] = mY.get() + mHeight.get();

                glBindBuffer(GL_ARRAY_BUFFER, _vboId);
                glBufferData(GL_ARRAY_BUFFER, sizeof (vertexData), vertexData, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            void draw() {
                glBindBuffer(GL_ARRAY_BUFFER, _vboId);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0,0);

                glDrawArrays(GL_TRIANGLES, 0, 6);

                glDisableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }


            //properties
            utils::Property<float> mX{_x};
            utils::Property<float> mY{_y};
            utils::Property<float> mWidth{_mWidth};
            utils::Property<float> mHeight{_mHeight};
        private:
            float _x, _y;
            float _mWidth, _mHeight;
            GLuint _vboId = 0;
        };

    }

    enum class AppState {
        PLAY, QUIT
    };

    class LoliApp {
    public:
        explicit LoliApp(utils::ILogger* logger, const std::string& name = "Loli Engine") {
            _pWindow = nullptr;
            _mLogger = logger;
            _mCurrentState = AppState::PLAY;
            _sName = name;

        }
        LoliApp& run() {
            init();
            _mSprite.init(-1.0f, -1.0f, 0.2f, 0.2f);
            gameLoop();
            return *this;
        }

        // properties
        LoliApp& screenWidth(uint16_t value) {
            if (value != _uScreenWidth) {
                _uScreenWidth = value;
            }
            return *this;
        }
        [[nodiscard]] uint16_t screenWidth() const {
            return _uScreenWidth;
        }

        LoliApp& screenHeight(uint16_t value) {
            if (value != _uScreenHeight) {
                _uScreenHeight = value;
            }
            return *this;
        }

        [[nodiscard]] uint16_t screenHeight() const {
            return _uScreenHeight;
        }

        LoliApp& changeStateTo(AppState state) {
            if (_mCurrentState != state) {
                _mCurrentState = state;
                _mLogger->log("app state was changed");
            }
            return *this;
        }

        [[nodiscard]] AppState currentState() const {
            return _mCurrentState;
        }

        // overrides
        virtual LoliApp& OnKeyDown (SDL_Keycode key) {
            switch (key) {
                case SDLK_ESCAPE:
                    end();
                    break;
            }
            return *this;
        }
    private:
        LoliApp& init() {
            _mLogger->log("engine initializing...");

            SDL_Init(SDL_INIT_EVERYTHING);
            _pWindow = SDL_CreateWindow(
                    _sName.c_str(),
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    screenHeight(), screenWidth(),
                    SDL_WINDOW_OPENGL);
            if (_pWindow == nullptr) {
                _mLogger->log("SDL window was not created");
                SDL_Quit();
            }

            SDL_GLContext context = SDL_GL_CreateContext(_pWindow);
            if (context == nullptr) {
                _mLogger->log("SDL window was not created");
                SDL_Quit();
            }
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            glClearColor(0.0,0.0,0.0, 1.0);

            return *this;
        }

        LoliApp& processInput () {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_KEYDOWN:
                        OnKeyDown(event.key.keysym.sym);
                        break;
                    case SDL_QUIT:
                        end();
                            break;
                }
            }
            return *this;
        }

        void draw() {
            glClearDepth(1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            _mSprite.draw();

            SDL_GL_SwapWindow(_pWindow);
        }
        void gameLoop() {
            _mLogger->log("enter into game loop...");
            while (currentState() != AppState::QUIT) {
                processInput();
                draw();
            }
            _mLogger->log("exiting game loop.");
        }
        void end() {
            SDL_DestroyWindow(_pWindow);
            changeStateTo(AppState::QUIT);
        }
    private:
        SDL_Window *_pWindow;

        uint16_t _uScreenWidth = 800;
        uint16_t _uScreenHeight = 600;

        std::string _sName = "";

        AppState _mCurrentState;

        //utility
        utils::ILogger* _mLogger;


        // For Delete
        graphics::Sprite _mSprite{};
    };
}
#endif //LOLIENGINE_LOLIENGINE_HPP
