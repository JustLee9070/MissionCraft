
#include "../include/screens/TitleScreen.h"

void TitleScreen::handleEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN &&
        event.key.key == SDLK_RETURN) {
        requestScreenChange(ScreenType::MainMenu);
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event.button.button == SDL_BUTTON_LEFT) {
        requestScreenChange(ScreenType::MainMenu);
    }
}

void TitleScreen::update(float deltaTime) {
    (void)deltaTime;
}

void TitleScreen::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 8, 15, 35, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 180, 60, 255);
    SDL_RenderDebugText(renderer, 480, 280, "MISSIONCRAFT");

    SDL_SetRenderDrawColor(renderer, 220, 225, 235, 255);
    SDL_RenderDebugText(renderer, 440, 330, "DESIGN. ENGINEER. EXPLORE.");

    SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
    SDL_RenderDebugText(renderer, 490, 430, "CLICK TO CONTINUE");
}
