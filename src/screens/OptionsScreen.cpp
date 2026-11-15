
#include "../include/screens/OptionsScreen.h"
#include "SpaceArtwork.h"
#include <algorithm>
#include <cstdio>
#include <utility>

namespace {
constexpr SDL_FRect fullscreenButton{400, 250, 400, 54};
constexpr SDL_FRect vsyncButton{400, 320, 400, 54};
constexpr SDL_FRect fpsButton{400, 386, 400, 54};

constexpr SDL_FRect masterQuieterButton{400, 450, 83, 44};
constexpr SDL_FRect masterLouderButton{717, 450, 83, 44};

constexpr SDL_FRect musicQuieterButton{400, 502, 83, 44};
constexpr SDL_FRect musicLouderButton{717, 502, 83, 44};

constexpr SDL_FRect resetButton{400, 560, 400, 54};
constexpr SDL_FRect backButton{400, 625, 170, 54};
constexpr SDL_FRect applyButton{630, 625, 170, 54};
}

OptionsScreen::OptionsScreen(DisplaySettings settings, ApplySettings applySettings)
    : draft(settings), apply(std::move(applySettings))
{
}

void OptionsScreen::handleEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        requestScreenChange(ScreenType::MainMenu);
        return;
    }

    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT) {
        return;
    }

    const SDL_FPoint click{event.button.x, event.button.y};
    status = "";

    if (SDL_PointInRectFloat(&click, &fullscreenButton)) {
        draft.fullscreen = !draft.fullscreen;
    } else if (SDL_PointInRectFloat(&click, &vsyncButton)) {
        draft.vsync = !draft.vsync;
    } else if (SDL_PointInRectFloat(&click, &fpsButton)) {
        draft.showFPS = !draft.showFPS;
    } else if (SDL_PointInRectFloat(&click, &masterQuieterButton)) {
        draft.masterVolume = std::max(0, draft.masterVolume - 10);
    } else if (SDL_PointInRectFloat(&click, &masterLouderButton)) {
        draft.masterVolume = std::min(100, draft.masterVolume + 10);
    } else if (SDL_PointInRectFloat(&click, &musicQuieterButton)) {
        draft.musicVolume = std::max(0, draft.musicVolume - 10);
    } else if (SDL_PointInRectFloat(&click, &musicLouderButton)) {
        draft.musicVolume = std::min(100, draft.musicVolume + 10);
    } else if (SDL_PointInRectFloat(&click, &resetButton)) {
        const DisplaySettings defaults;
        draft.fullscreen = defaults.fullscreen;
        draft.vsync = defaults.vsync;
        draft.showFPS = defaults.showFPS;
        draft.masterVolume = defaults.masterVolume;
        draft.musicVolume = defaults.musicVolume;
    } else if (SDL_PointInRectFloat(&click, &backButton)) {
        requestScreenChange(ScreenType::MainMenu);
    } else if (SDL_PointInRectFloat(&click, &applyButton)) {
        status = apply(draft) ? "Settings saved." : "Could not apply or save settings.";
    }
}

void OptionsScreen::update(float deltaTime) {
    (void)deltaTime;
}

void OptionsScreen::render(SDL_Renderer* renderer) {
    using namespace screenDrawing;

    const auto mouse = pointerPosition(renderer);
    spaceArtwork::drawBackdrop(renderer);

    drawText(renderer, 400, 155, "OPTIONS", text, 3.5f);

    drawButton(renderer, fullscreenButton, mouse,
               draft.fullscreen ? "Fullscreen: On" : "Fullscreen: Off", true);

    drawButton(renderer, vsyncButton, mouse,
               draft.vsync ? "VSync: On" : "VSync: Off", true);

    drawButton(renderer, fpsButton, mouse,
               draft.showFPS ? "Show FPS: On" : "Show FPS: Off", true);

    char volume[48];

    std::snprintf(volume, sizeof(volume), "Master volume %d%%", draft.masterVolume);
    drawText(renderer, 498, 460, volume, text, 1.5f);

    drawButton(renderer, masterQuieterButton, mouse, "-10", draft.masterVolume > 0);
    drawButton(renderer, masterLouderButton, mouse, "+10", draft.masterVolume < 100);

    std::snprintf(volume, sizeof(volume), "Music volume %d%%", draft.musicVolume);
    drawText(renderer, 498, 512, volume, text, 1.5f);

    drawButton(renderer, musicQuieterButton, mouse, "-10", draft.musicVolume > 0);
    drawButton(renderer, musicLouderButton, mouse, "+10", draft.musicVolume < 100);

    drawButton(renderer, resetButton, mouse, "Reset settings", true);
    drawButton(renderer, backButton, mouse, "Back", true);
    drawButton(renderer, applyButton, mouse, "Apply", true, true);

    drawText(renderer, 400, 690, status, muted, 1.5f);
}
