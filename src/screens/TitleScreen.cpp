
#include "../include/screens/TitleScreen.h"
#include "SpaceArtwork.h"

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
    using namespace screenDrawing;
    spaceArtwork::drawBackdrop(renderer, spaceArtwork::nebula, 165);
    spaceArtwork::drawSprite(renderer, spaceArtwork::sun, {1080, -115, 228, 228}, 185);
    spaceArtwork::drawSprite(renderer, spaceArtwork::earth, {635, 158, 485, 485}, 218);
    spaceArtwork::drawSprite(renderer, spaceArtwork::moon, {1020, 156, 112, 112});
    spaceArtwork::drawSprite(renderer, spaceArtwork::ship, {615, 438, 214, 214});
    setColor(renderer, cyan);
    SDL_RenderLine(renderer, 62, 296, 132, 296);
    drawText(renderer, 62, 318, "MISSIONCRAFT", text, 5.5f);
    drawText(renderer, 64, 414, "Design. Engineer. Explore.", text, 1.75f);
    drawButton(renderer, {62, 485, 264, 55}, pointerPosition(renderer),
               "Continue (Enter)", true, true);
}
