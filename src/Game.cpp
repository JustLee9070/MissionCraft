
#include "Game.h"
#include "screens/TitleScreen.h"
#include "screens/MainMenu.h"
#include "screens/DesignerScreen.h"
#include "screens/PlannerScreen.h"
#include "screens/MissionControl.h"
#include "screens/OptionsScreen.h"
#include "screens/ScreenDrawing.h"

#include <iostream>
#include <utility>

namespace
{
    constexpr int DESIGN_WIDTH = 1200;
    constexpr int DESIGN_HEIGHT = 800;
    constexpr SDL_FRect saveButton{1008, 764, 160, 32};

    bool canSaveOn(ScreenType screen)
    {
        return screen == ScreenType::Designer || screen == ScreenType::Planner ||
               screen == ScreenType::MissionControl;
    }

    bool saveRequested(const SDL_Event& event)
    {
        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
            return event.key.key == SDLK_F5;
        }
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            const SDL_FPoint click{event.button.x, event.button.y};
            return SDL_PointInRectFloat(&click, &saveButton);
        }
        return false;
    }
}

bool Game::initialize()
{
    options.load();
    const auto& display = options.getSettings();

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << '\n';
        return false;
    }

    if (!SDL_CreateWindowAndRenderer(
            "MissionCraft",
            display.width,
            display.height,
            display.fullscreen ? SDL_WINDOW_FULLSCREEN : 0,
            &window,
            &renderer))
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << '\n';
        return false;
    }

    if (!SDL_SetRenderLogicalPresentation(renderer, DESIGN_WIDTH, DESIGN_HEIGHT,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        std::cerr << "Could not configure the game viewport: " << SDL_GetError() << '\n';
        return false;
    }

    if (!SDL_SetRenderVSync(renderer, display.vsync ? 1 : 0)) {
        std::cerr << "Could not apply saved VSync setting: " << SDL_GetError() << '\n';
    }

    if (!TTF_Init())
    {
        std::cerr << "SDL_ttf initialization failed: " << SDL_GetError() << '\n';
        return false;
    }

    ttfInitialized = true;
    assets = std::make_unique<AssetManager>(renderer);

    audio = std::make_unique<GameAudio>();

    if (audio->initialize()) {
        audio->setVolume(display.masterVolume);
        audio->setMusicVolume(display.musicVolume);

        if (!audio->playMusic("assets/audio/music/space_ambient_converted.wav")) {
            std::cerr << "Could not load background music.\n";
        }
    } else {
        std::cerr << "Audio initialization failed: " << SDL_GetError() << '\n';
    }

    running = true;
    changeScreen(ScreenType::Title);

    return true;
}

bool Game::applyDisplaySettings(const DisplaySettings& requested)
{
    const auto previous = options.getSettings();
    int previousVsync = 0;

    if (!SDL_GetRenderVSync(renderer, &previousVsync)) {
        return false;
    }

    const bool wasFullscreen = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;

    if (!SDL_SetRenderVSync(renderer, requested.vsync ? 1 : 0)) {
        return false;
    }

    if (wasFullscreen != requested.fullscreen &&
        !SDL_SetWindowFullscreen(window, requested.fullscreen)) {
        SDL_SetRenderVSync(renderer, previousVsync);
        return false;
    }

    options.setSettings(requested);

    if (options.save()) {
        if (audio) {
            const auto& settings = options.getSettings();
            audio->setVolume(settings.masterVolume);
            audio->setMusicVolume(settings.musicVolume);
            audio->play(SoundCue::Select);
        }
        return true;
    }

    options.setSettings(previous);
    SDL_SetWindowFullscreen(window, wasFullscreen);
    SDL_SetRenderVSync(renderer, previousVsync);

    return false;
}

void Game::changeScreen(ScreenType screen)
{
    const char* menuMessage = "";

    if (screen == ScreenType::LoadGame) {
        if (auto saved = saves.loadGame()) {
            gameState = std::move(saved->state);
            screen = saved->screen;
        } else {
            menuMessage = "Could not load the saved game.";
            screen = ScreenType::MainMenu;
        }
    } else if (screen == ScreenType::NewGame) {
        gameState = GameState{};
        screen = ScreenType::Designer;
    }

    activeScreen = screen;
    saveMessage = "";

    switch (screen)
    {
    case ScreenType::Title:
        currentScreen = std::make_unique<TitleScreen>();
        break;

    case ScreenType::MainMenu:
        currentScreen = std::make_unique<MainMenu>(saves.hasSave(), menuMessage);
        break;

    case ScreenType::Designer:
        currentScreen = std::make_unique<DesignerScreen>(gameState);
        break;

    case ScreenType::Planner:
        currentScreen = std::make_unique<PlannerScreen>(gameState);
        break;

    case ScreenType::MissionControl:
        currentScreen = std::make_unique<MissionControl>(gameState);
        break;

    case ScreenType::Options:
        currentScreen = std::make_unique<OptionsScreen>(
            options.getSettings(), [this](const DisplaySettings& settings) {
                return applyDisplaySettings(settings);
            });
        break;

    case ScreenType::Exit:
        running = false;
        currentScreen.reset();
        return;

    case ScreenType::NewGame:
    case ScreenType::LoadGame:
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
            SDL_ConvertEventToRenderCoordinates(renderer, &event);

            if (canSaveOn(activeScreen) && saveRequested(event)) {
                saveMessage = saves.saveGame(gameState, activeScreen) ?
                    "Game saved." : "Could not save the game.";
            } else {
                const auto previousPhase = gameState.missionPhase();
                const auto previousSteps = gameState.completedSteps();
                const auto previousParts = gameState.spacecraft().components().size();
                const auto previousDestination = gameState.destination().id;

                currentScreen->handleEvents(event);

                if (audio) {
                    const auto phase = gameState.missionPhase();

                    if (phase != previousPhase && phase == MissionPhase::Completed) {
                        audio->play(SoundCue::Complete);
                    } else if (phase != previousPhase && phase == MissionPhase::Failed) {
                        audio->play(SoundCue::Failure);
                    } else if (activeScreen == ScreenType::MissionControl &&
                               previousSteps == 0 && gameState.completedSteps() == 1) {
                        audio->play(SoundCue::Launch);
                    } else if (hasPendingScreen ||
                               previousParts != gameState.spacecraft().components().size() ||
                               previousDestination != gameState.destination().id) {
                        audio->play(SoundCue::Select);
                    }
                }
            }
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
        const auto previousPhase = gameState.missionPhase();

        currentScreen->update(deltaTime);

        if (audio && previousPhase == MissionPhase::InProgress) {
            if (gameState.missionPhase() == MissionPhase::Completed) {
                audio->play(SoundCue::Complete);
            } else if (gameState.missionPhase() == MissionPhase::Failed) {
                audio->play(SoundCue::Failure);
            }
        }
    }
}

void Game::render()
{
    if (!currentScreen)
    {
        return;
    }

    currentScreen->render(renderer);

    if (canSaveOn(activeScreen)) {
        using namespace screenDrawing;
        drawText(renderer, 32, 770, saveMessage, muted, 1.25f);
        drawButton(renderer, saveButton, pointerPosition(renderer), "Save (F5)", true);
    }

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
    assets.reset();
    audio.reset();

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
