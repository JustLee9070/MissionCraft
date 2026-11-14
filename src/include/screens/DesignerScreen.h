#pragma once
#include "../Screen.h"
#include "../GameState.h"
#include <optional>

class DesignerScreen : public Screen {
public:
    explicit DesignerScreen(GameState& gameState);
    void handleEvents(const SDL_Event& event) override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;

private:
    void addPart(ComponentId id);
    void removeSelectedPart();
    void openPlanner();

    GameState& state;
    std::optional<ComponentId> selectedPart;
    std::array<float, 7> installTime{};
    std::array<float, 7> slotProgress{};
};
