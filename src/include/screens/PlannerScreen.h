#pragma once
#include "../Screen.h"
#include "../GameState.h"

class PlannerScreen : public Screen {
public:
    explicit PlannerScreen(GameState& gameState);
    void handleEvents(const SDL_Event& event) override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;

private:
    void startMission();
    GameState& state;
};
