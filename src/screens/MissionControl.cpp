
#include "../include/screens/MissionControl.h"

void MissionControl::handleEvents(const SDL_Event& event) {
    (void)event;
}

void MissionControl::update(float deltaTime) {
    (void)deltaTime;
}

void MissionControl::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 10, 20, 40, 255);
    SDL_RenderClear(renderer);
}
