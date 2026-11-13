
#pragma once
#include <SDL3/SDL.h>
#include <functional>

enum class ScreenType {
    Title,
    MainMenu,
    Designer,
    Planner,
    MissionControl,
    Options,
    Exit
};

class Screen {
private:
    std::function<void(ScreenType)> screenChangeCallback;

protected:
    void requestScreenChange(ScreenType screen) {
        if (screenChangeCallback) {
            screenChangeCallback(screen);
        }
    }

public:
    virtual ~Screen() = default;

    void setScreenChangeCallback(std::function<void(ScreenType)> callback) {
        screenChangeCallback = callback;
    }

    virtual void handleEvents(const SDL_Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;
};
