
#include "../include/screens/OptionsScreen.h"

void OptionsScreen::handleEvents(const SDL_Event& event) {
    (void)event;
}

void OptionsScreen::update(float deltaTime) {
    (void)deltaTime;
}

void OptionsScreen::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
    SDL_RenderClear(renderer);
}
