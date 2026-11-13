#pragma once
#include "../Screen.h"

class MainMenu : public Screen {
public:
    void handleEvents(const SDL_Event& event) override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
};
