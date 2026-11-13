#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>

class Game {
private:
    enum class Screen {
        Designer,
        Planner,
        MissionControl
    };

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool running = false;

    Screen currentScreen = Screen::Designer;

    std::vector<std::string> components;
    std::string destination = "Moon";

    float totalMass = 0.0f;
    float powerGeneration = 0.0f;
    float powerConsumption = 0.0f;
    float fuelCapacity = 0.0f;

    int missionPhase = 0;

    void handleEvents();
    void render();
    void renderDesigner();
    void renderPlanner();
    void renderMissionControl();
    void addComponent(int componentType);
    bool isSpacecraftReady() const;

public:
    bool initialize();
    void run();
    ~Game();
};