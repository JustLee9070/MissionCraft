
#include "../include/screens/MainMenu.h"

namespace {
    struct MenuButton {
        SDL_FRect rect;
        const char* label;
        ScreenType destination;
    };

    MenuButton buttons[] = {
        {{450, 300, 300, 55}, "NEW GAME", ScreenType::Designer},
        {{450, 375, 300, 55}, "LOAD GAME", ScreenType::MainMenu},
        {{450, 450, 300, 55}, "OPTIONS", ScreenType::Options},
        {{450, 525, 300, 55}, "EXIT", ScreenType::Exit}
    };

    bool isInside(float x, float y, const SDL_FRect& rect) {
        return x >= rect.x && x <= rect.x + rect.w &&
               y >= rect.y && y <= rect.y + rect.h;
    }
}

void MainMenu::handleEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event.button.button == SDL_BUTTON_LEFT) {

        for (const auto& button : buttons) {
            if (isInside(event.button.x, event.button.y, button.rect)) {
                if (button.label != nullptr) {
                    requestScreenChange(button.destination);
                }
                break;
            }
        }
    }
}

void MainMenu::update(float deltaTime) {
    (void)deltaTime;
}

void MainMenu::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 12, 20, 40, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 180, 60, 255);
    SDL_RenderDebugText(renderer, 540, 200, "MISSIONCRAFT");

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    for (const auto& button : buttons) {
        bool hovered = isInside(mouseX, mouseY, button.rect);

        if (hovered) {
            SDL_SetRenderDrawColor(renderer, 190, 120, 35, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 45, 60, 85, 255);
        }

        SDL_RenderFillRect(renderer, &button.rect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        SDL_RenderDebugText(
            renderer,
            button.rect.x + 115,
            button.rect.y + 23,
            button.label
        );
    }
}
