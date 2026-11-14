#pragma once
#include "../Screen.h"
#include "../OptionsManager.h"

class OptionsScreen : public Screen {
public:
    using ApplySettings = std::function<bool(const DisplaySettings&)>;
    OptionsScreen(DisplaySettings settings, ApplySettings apply);
    void handleEvents(const SDL_Event& event) override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;

private:
    DisplaySettings draft;
    ApplySettings apply;
    const char* status = "";
};
