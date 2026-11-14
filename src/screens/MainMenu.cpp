#include "screens/MainMenu.h"
#include "SpaceArtwork.h"

namespace {
using namespace screenDrawing;

struct MenuButton {
    SDL_FRect bounds;
    const char* label;
    ScreenType destination;
};

constexpr MenuButton buttons[] = {
    {{450, 300, 300, 55}, "New game", ScreenType::NewGame},
    {{450, 375, 300, 55}, "Load game", ScreenType::LoadGame},
    {{450, 450, 300, 55}, "Options", ScreenType::Options},
    {{450, 525, 300, 55}, "Exit", ScreenType::Exit}
};
}

void MainMenu::handleEvents(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT) {
        return;
    }

    const SDL_FPoint click{event.button.x, event.button.y};
    for (const auto& button : buttons) {
        if ((saveAvailable || button.destination != ScreenType::LoadGame) &&
            SDL_PointInRectFloat(&click, &button.bounds)) {
            requestScreenChange(button.destination);
            return;
        }
    }
}

void MainMenu::update(float deltaTime)
{
    (void)deltaTime;
}

void MainMenu::render(SDL_Renderer* renderer)
{
    const auto mouse = pointerPosition(renderer);
    spaceArtwork::drawBackdrop(renderer, spaceArtwork::nebula, 165);
    spaceArtwork::drawSprite(renderer, spaceArtwork::earth, {-145, 280, 540, 540}, 190);
    spaceArtwork::drawSprite(renderer, spaceArtwork::moon, {885, 160, 230, 230}, 212);
    drawText(renderer, 450, 192, "MISSIONCRAFT", text, 3.75f);
    for (const auto& button : buttons) {
        drawButton(renderer, button.bounds, mouse, button.label,
                   saveAvailable || button.destination != ScreenType::LoadGame);
    }
    drawText(renderer, 450, 602, message, amber, 1.5f);
}
