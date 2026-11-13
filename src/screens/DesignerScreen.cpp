
#include "../include/screens/DesignerScreen.h"

void DesignerScreen::handleEvents(const SDL_Event& event) {
    (void)event;
}

void DesignerScreen::update(float deltaTime) {
    (void)deltaTime;
}

void DesignerScreen::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 20, 25, 35, 255);
    SDL_RenderClear(renderer);
}
