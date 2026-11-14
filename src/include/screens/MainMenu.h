#pragma once
#include "../Screen.h"

class MainMenu : public Screen {
public:
    explicit MainMenu(bool saveAvailable = false, const char* message = "")
        : saveAvailable(saveAvailable), message(message) {}

    void handleEvents(const SDL_Event& event) override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;

private:
    bool saveAvailable;
    const char* message;
};
