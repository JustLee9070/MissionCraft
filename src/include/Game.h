#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include "OptionsManager.h"
#include "GameState.h"
#include "Screen.h"
#include "AssetManager.h"
#include "SaveManager.h"
#include "GameAudio.h"

class Game {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    bool running = false;
    bool ttfInitialized = false;
    OptionsManager options;
    GameState gameState;
    SaveManager saves;
    std::unique_ptr<GameAudio> audio;
    std::unique_ptr<AssetManager> assets;

    std::unique_ptr<Screen> currentScreen;
    ScreenType activeScreen = ScreenType::Title;
    const char* saveMessage = "";

    ScreenType pendingScreen = ScreenType::Title;
    bool hasPendingScreen = false;

    void handleEvents();
    void update(float deltaTime);
    void render();
    void changeScreen(ScreenType screen);
    bool applyDisplaySettings(const DisplaySettings& requested);

public:
    bool initialize();
    void run();
    ~Game();
};
