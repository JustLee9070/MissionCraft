
#include "Game.h"

#include "screens/TitleScreen.h"
#include "screens/MainMenu.h"
#include "screens/DesignerScreen.h"
#include "screens/PlannerScreen.h"
#include "screens/MissionControl.h"
#include "screens/OptionsScreen.h"

#include <iostream>

namespace
{
    constexpr int WINDOW_WIDTH = 1200;
    constexpr int WINDOW_HEIGHT = 800;
}

bool Game::initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << '\n';
        return false;
    }

    if (!SDL_CreateWindowAndRenderer(
            "MissionCraft",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            0,
            &window,
            &renderer))
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << '\n';
        return false;
    }

    SDL_SetRenderVSync(renderer, 1);

    if (!TTF_Init())
    {
        std::cerr << "SDL_ttf initialization failed: " << SDL_GetError() << '\n';
        return false;
    }

    ttfInitialized = true;
    running = true;

    changeScreen(ScreenType::Title);

    return true;
}

void Game::changeScreen(ScreenType screen)
{
    switch (screen)
    {
    case ScreenType::Title:
        currentScreen = std::make_unique<TitleScreen>();
        break;

    case ScreenType::MainMenu:
        currentScreen = std::make_unique<MainMenu>();
        break;

    case ScreenType::Designer:
        currentScreen = std::make_unique<DesignerScreen>();
        break;

    case ScreenType::Planner:
        currentScreen = std::make_unique<PlannerScreen>();
        break;

    case ScreenType::MissionControl:
        currentScreen = std::make_unique<MissionControl>();
        break;

    case ScreenType::Options:
        currentScreen = std::make_unique<OptionsScreen>();
        break;

    case ScreenType::Exit:
        running = false;
        currentScreen.reset();
        return;
    }

    if (currentScreen)
    {
        currentScreen->setScreenChangeCallback(
            [this](ScreenType nextScreen)
            {
                pendingScreen = nextScreen;
                hasPendingScreen = true;
            }
        );
    }
}

void Game::handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
            break;
        }

        if (currentScreen)
        {
            currentScreen->handleEvents(event);
        }

        if (hasPendingScreen)
        {
            break;
        }
    }

    if (hasPendingScreen)
    {
        hasPendingScreen = false;
        changeScreen(pendingScreen);
    }
}

void Game::update(float deltaTime)
{
    if (currentScreen)
    {
        currentScreen->update(deltaTime);
    }
}

void Game::render()
{
    if (!currentScreen)
    {
        return;
    }

    currentScreen->render(renderer);
    SDL_RenderPresent(renderer);
}

void Game::run()
{
    Uint64 lastTime = SDL_GetTicks();

    while (running)
    {
        Uint64 currentTime = SDL_GetTicks();

        float deltaTime = static_cast<float>(currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        handleEvents();
        update(deltaTime);
        render();
    }
}

Game::~Game()
{
    currentScreen.reset();

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }

    if (window)
    {
        SDL_DestroyWindow(window);
    }

    if (ttfInitialized)
    {
        TTF_Quit();
    }

    SDL_Quit();
}
