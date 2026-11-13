#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include "Screen.h"

class Game {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool running = false;
    bool ttfInitialized = false;

    std::unique_ptr<Screen> currentScreen;

    ScreenType pendingScreen = ScreenType::Title;
    bool hasPendingScreen = false;

    void handleEvents();
    void update(float deltaTime);
    void render();
    void changeScreen(ScreenType screen);

public:
    bool initialize();
    void run();
    ~Game();
};