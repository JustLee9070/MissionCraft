
#include "../include/screens/PlannerScreen.h"

void PlannerScreen::handleEvents(const SDL_Event& event) {
    (void)event;
}

void PlannerScreen::update(float deltaTime) {
    (void)deltaTime;
}

void PlannerScreen::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 15, 30, 45, 255);
    SDL_RenderClear(renderer);
}
