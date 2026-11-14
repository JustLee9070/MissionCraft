#pragma once
#include "../Screen.h"
#include "../GameState.h"

class MissionControl : public Screen {
public:
    explicit MissionControl(GameState& gameState);
    void handleEvents(const SDL_Event& event) override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;

private:
    GameState& state;
    float flightProgress = 0;
    float fallProgress = 0;
};
