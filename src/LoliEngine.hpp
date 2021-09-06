//
// Created by danielf on 05.09.2021.
//

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GLES3/gl3.h>

#include <vector>
#include <string>
#include <concepts>
#include <functional>
#include <ranges>

#ifndef LOLIENGINE_LOLIENGINE_HPP
#define LOLIENGINE_LOLIENGINE_HPP

namespace loli {
    namespace utils {
        struct ILogger {
            virtual void log (const std::string& msg) = 0;
        };

        struct ConsoleLogger : public ILogger {
            void log(const std::string& msg) override {
                std::cout << "["<<__BASE_FILE__ <<":"<<__LINE__<<"]: " << msg << std::endl;
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
        namespace args {
            struct IEventArgs{};
            template<typename T>
            concept EventArguments = std::derived_from<T, IEventArgs> || std::is_reference_v<T>;

            struct KeyDownEventArgs : public IEventArgs {
                explicit KeyDownEventArgs(SDL_Keycode keyCode) {
                    code.set(keyCode);
                }
                //TODO: change it
                utils::Property<SDL_Keycode> code{_mCode};
            private:
                SDL_Keycode _mCode{};
            };

            struct EmptyEventArgs : public IEventArgs{};
        };
        struct ISubscriber{};

        template<typename T>
        concept Subscriber = std::derived_from<T, ISubscriber> || std::is_reference_v<T>;

        template<typename TSender, typename TEventArgs>
            requires Subscriber<TSender> && args::EventArguments<TEventArgs>
        struct Event {
            struct Subscription {
               explicit Subscription(ISubscriber* sub) : _mSubscriber(sub) {}
               virtual ~Subscription() = default;

                Subscription* add (std::function<void (const TSender, const TEventArgs)> func) {
                    _mFunction = func;
                    return this;
                }

                void invoke (const TEventArgs eventArgs) {
                    _mFunction(static_cast<TSender>(*_mSubscriber), eventArgs);
                }
                ISubscriber* _mSubscriber = nullptr;
            private:
                std::function<void(const TSender, const TEventArgs)> _mFunction = nullptr;
            };

            Subscription* subscribe(ISubscriber* subscriber) {
                auto *sub = new Subscription(subscriber);
                _vSubscriptions.push_back(sub);
                return sub;
            }

            void remove (Subscription* sub) {
                _vSubscriptions.erase(std::remove_if(_vSubscriptions.begin(), _vSubscriptions.end(), [&](auto elem) {
                   return sub == elem;
                }));
            }

            void invoke (TEventArgs eventArgs) {
                for (const auto& subscription : _vSubscriptions) {
                   subscription->invoke (eventArgs);
                }
            }
        private:
            std::vector<Subscription*> _vSubscriptions{};
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


        struct Window  : public events::ISubscriber {
            virtual void screenWidth(uint16_t value) {
                if (value != _uScreenWidth) {
                    _uScreenWidth = value;
                }
            }
            [[nodiscard]] virtual uint16_t screenWidth() const {
                return _uScreenWidth;
            }

            virtual void screenHeight(uint16_t value) {
                if (value != _uScreenHeight) {
                    _uScreenHeight = value;
                }
            }

            [[nodiscard]] virtual uint16_t screenHeight() const {
                return _uScreenHeight;
            }

            virtual void name (const std::string& name) {
                if (_sName == name) {
                    return;
                }
                _sName = name;
            }

            [[nodiscard]] virtual std::string name() const {
                return _sName;
            }

            virtual void init() = 0;
            virtual void draw() = 0;
            virtual void destroy() = 0;

            events::Event<Window&, events::args::IEventArgs&> OnDraw{};

        private:
            uint16_t _uScreenWidth = 800;
            uint16_t _uScreenHeight = 600;

            std::string _sName = "";

        };

        class SDLWindow : public Window {
        public:
            explicit SDLWindow (const std::string& name, utils::ILogger* logger = new utils::ConsoleLogger) {
                this->name(name);
                _mLogger = logger;
            }
        private:
            SDL_Window *_mWindow = nullptr;
            utils::ILogger* _mLogger;
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

            void draw() override {
                glClearDepth(1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // TODO: create OnDrawEventArgs und pass it here
                events::args::EmptyEventArgs eventArgs;
                OnDraw.invoke(eventArgs);
                SDL_GL_SwapWindow(_mWindow);
            }

            void destroy () override {
                SDL_DestroyWindow(_mWindow);
            }
        };
    }

    enum class AppState {
        PLAY, QUIT
    };

    class LoliApp : public events::ISubscriber {
    public:
        explicit LoliApp(utils::ILogger* logger, const std::string& name = "Loli Engine") {
            _mLogger = logger;
            _mCurrentState = AppState::PLAY;

            KeyDownEvent.subscribe(this)->add([&](auto& s, auto &e) {
                OnKeyDown(e.code.get());
            });
        }
        LoliApp& run() {
            init();
            _mSprite.init(-1.0f, -1.0f, 0.2f, 0.2f);
            gameLoop();
            return *this;
        }

        // properties

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
    public:
        events::Event<LoliApp&, events::args::KeyDownEventArgs&> KeyDownEvent;
    private:
        LoliApp& init() {
            _mLogger->log("engine initializing...");
            _mWindow->init();

            return *this;
        }

        LoliApp& processInput () {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        end();
                        break;
                    case SDL_KEYDOWN:
                        events::args::KeyDownEventArgs eventArgs(event.key.keysym.sym);
                        KeyDownEvent.invoke(eventArgs);
                        break;
                }
            }
            return *this;
        }

        void draw() {
            _mWindow->draw();
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
            _mWindow->destroy();
            changeStateTo(AppState::QUIT);
        }
    private:

        /*
         * App Window
         * */
        graphics::Window* _mWindow = nullptr;

        AppState _mCurrentState;

        //TODO: Delete
        graphics::Sprite _mSprite{};
    protected:
        //utility
        utils::ILogger* _mLogger;

    };
}
#endif //LOLIENGINE_LOLIENGINE_HPP
