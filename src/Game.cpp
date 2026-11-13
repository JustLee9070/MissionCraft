
#include "./include/Game.h"
#include <iostream>

namespace {
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 800;

    const char* missionPhases[] = {
        "Launch",
        "Earth Orbit",
        "Transfer Burn",
        "Deep Space Cruise",
        "Destination Approach",
        "Mission Complete"
    };

    void setColor(SDL_Renderer* renderer, int r, int g, int b, int a = 255) {
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }

    void drawText(SDL_Renderer* renderer, float x, float y, const char* text,
                  int r = 230, int g = 230, int b = 230) {
        setColor(renderer, r, g, b);
        SDL_RenderDebugText(renderer, x, y, text);
    }

    void drawButton(SDL_Renderer* renderer, float x, float y, float w, float h,
                    const char* text, bool active = false) {
        SDL_FRect rect = {x, y, w, h};

        if (active) {
            setColor(renderer, 220, 130, 35);
        } else {
            setColor(renderer, 45, 48, 55);
        }

        SDL_RenderFillRect(renderer, &rect);

        setColor(renderer, 100, 105, 115);
        SDL_RenderRect(renderer, &rect);

        drawText(renderer, x + 12, y + h / 2 - 4, text);
    }

    bool isInside(float x, float y, float bx, float by, float bw, float bh) {
        return x >= bx && x <= bx + bw && y >= by && y <= by + bh;
    }
}

Game::~Game() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

bool Game::initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << '\n';
        return false;
    }

    if (!SDL_CreateWindowAndRenderer(
        "MissionCraft - Design. Engineer. Explore.",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE,
        &window,
        &renderer
    )) {
        std::cerr << "Window creation failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return false;
    }

    SDL_SetRenderVSync(renderer, 1);
    running = true;

    std::cout << "MissionCraft initialized.\n";
    return true;
}

void Game::run() {
    while (running) {
        handleEvents();
        render();
    }
}

void Game::handleEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE) {
                running = false;
            }

            if (event.key.key == SDLK_1) {
                currentScreen = Screen::Designer;
            }

            if (event.key.key == SDLK_2) {
                currentScreen = Screen::Planner;
            }

            if (event.key.key == SDLK_3) {
                currentScreen = Screen::MissionControl;
            }
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button == SDL_BUTTON_LEFT) {

            float x = event.button.x;
            float y = event.button.y;

            // Navigation
            if (isInside(x, y, 20, 75, 180, 45)) {
                currentScreen = Screen::Designer;
            } else if (isInside(x, y, 20, 130, 180, 45)) {
                currentScreen = Screen::Planner;
            } else if (isInside(x, y, 20, 185, 180, 45)) {
                currentScreen = Screen::MissionControl;
            }

            // Designer component buttons
            if (currentScreen == Screen::Designer) {
                if (isInside(x, y, 240, 180, 200, 45)) {
                    addComponent(0);
                } else if (isInside(x, y, 240, 235, 200, 45)) {
                    addComponent(1);
                } else if (isInside(x, y, 240, 290, 200, 45)) {
                    addComponent(2);
                } else if (isInside(x, y, 240, 345, 200, 45)) {
                    addComponent(3);
                } else if (isInside(x, y, 240, 400, 200, 45)) {
                    addComponent(4);
                } else if (isInside(x, y, 240, 455, 200, 45)) {
                    addComponent(5);
                } else if (isInside(x, y, 240, 510, 200, 45)) {
                    addComponent(6);
                } else if (isInside(x, y, 240, 570, 200, 45) &&
                           !components.empty()) {
                    components.pop_back();
                    totalMass = 0;
                    powerGeneration = 0;
                    powerConsumption = 0;
                    fuelCapacity = 0;

                    // Recalculate telemetry after removing a component
                    std::vector<std::string> remaining = components;
                    components.clear();

                    for (const auto& name : remaining) {
                        if (name == "Command Module") addComponent(0);
                        else if (name == "Chemical Engine") addComponent(1);
                        else if (name == "Fuel Tank") addComponent(2);
                        else if (name == "Solar Array") addComponent(3);
                        else if (name == "Battery Pack") addComponent(4);
                        else if (name == "High-Gain Antenna") addComponent(5);
                        else if (name == "Science Payload") addComponent(6);
                    }
                }
            }

            // Mission planner
            if (currentScreen == Screen::Planner) {
                if (isInside(x, y, 240, 190, 200, 45)) {
                    destination = "Moon";
                } else if (isInside(x, y, 240, 245, 200, 45)) {
                    destination = "Mars";
                } else if (isInside(x, y, 240, 300, 200, 45)) {
                    destination = "Earth Orbit";
                } else if (isInside(x, y, 240, 390, 200, 45) &&
                           isSpacecraftReady()) {
                    missionPhase = 0;
                    currentScreen = Screen::MissionControl;
                }
            }

            // Mission control
            if (currentScreen == Screen::MissionControl) {
                if (isInside(x, y, 240, 400, 220, 50) &&
                    isSpacecraftReady() &&
                    missionPhase < 5) {
                    missionPhase++;
                }
            }
        }
    }
}

void Game::addComponent(int componentType) {
    switch (componentType) {
        case 0:
            components.push_back("Command Module");
            totalMass += 1200;
            powerConsumption += 150;
            break;

        case 1:
            components.push_back("Chemical Engine");
            totalMass += 800;
            powerConsumption += 50;
            break;

        case 2:
            components.push_back("Fuel Tank");
            totalMass += 500;
            fuelCapacity += 1000;
            break;

        case 3:
            components.push_back("Solar Array");
            totalMass += 200;
            powerGeneration += 300;
            break;

        case 4:
            components.push_back("Battery Pack");
            totalMass += 150;
            break;

        case 5:
            components.push_back("High-Gain Antenna");
            totalMass += 100;
            powerConsumption += 30;
            break;

        case 6:
            components.push_back("Science Payload");
            totalMass += 250;
            powerConsumption += 70;
            break;

        default:
            break;
    }
}

bool Game::isSpacecraftReady() const {
    bool hasCommandModule = false;
    bool hasEngine = false;

    for (const auto& component : components) {
        if (component == "Command Module") {
            hasCommandModule = true;
        }

        if (component == "Chemical Engine") {
            hasEngine = true;
        }
    }

    return hasCommandModule &&
           hasEngine &&
           fuelCapacity > 0 &&
           powerGeneration >= powerConsumption;
}

void Game::render() {
    setColor(renderer, 18, 20, 25);
    SDL_RenderClear(renderer);

    // Header
    drawText(renderer, 25, 25, "MISSIONCRAFT", 247, 151, 40);
    drawText(renderer, 250, 25, "DESIGN. ENGINEER. EXPLORE.");
    drawText(renderer, 1050, 25, "PROTOTYPE BUILD", 150, 155, 165);

    // Sidebar
    SDL_FRect sidebar = {0, 60, 220, 740};
    setColor(renderer, 25, 28, 34);
    SDL_RenderFillRect(renderer, &sidebar);

    drawText(renderer, 25, 45, "NAVIGATION", 150, 155, 165);

    drawButton(renderer, 20, 75, 180, 45, "1. Spacecraft Designer",
               currentScreen == Screen::Designer);
    drawButton(renderer, 20, 130, 180, 45, "2. Mission Planner",
               currentScreen == Screen::Planner);
    drawButton(renderer, 20, 185, 180, 45, "3. Mission Control",
               currentScreen == Screen::MissionControl);

    if (currentScreen == Screen::Designer) {
        renderDesigner();
    } else if (currentScreen == Screen::Planner) {
        renderPlanner();
    } else {
        renderMissionControl();
    }

    drawText(renderer, 25, 770, "ESC: Quit | 1-3: Switch screens", 130, 135, 145);

    SDL_RenderPresent(renderer);
}

void Game::renderDesigner() {
    drawText(renderer, 250, 90, "SPACECRAFT DESIGNER", 247, 151, 40);
    drawText(renderer, 250, 125, "Select components to assemble your spacecraft.");

    drawText(renderer, 250, 155, "COMPONENT CATALOG", 150, 155, 165);

    drawButton(renderer, 240, 180, 200, 45, "Command Module");
    drawButton(renderer, 240, 235, 200, 45, "Chemical Engine");
    drawButton(renderer, 240, 290, 200, 45, "Fuel Tank");
    drawButton(renderer, 240, 345, 200, 45, "Solar Array");
    drawButton(renderer, 240, 400, 200, 45, "Battery Pack");
    drawButton(renderer, 240, 455, 200, 45, "High-Gain Antenna");
    drawButton(renderer, 240, 510, 200, 45, "Science Payload");
    drawButton(renderer, 240, 570, 200, 45, "Remove Last Component");

    // Spacecraft assembly
    drawText(renderer, 500, 155, "ASSEMBLY", 150, 155, 165);

    SDL_FRect assemblyPanel = {490, 180, 340, 400};
    setColor(renderer, 27, 30, 37);
    SDL_RenderFillRect(renderer, &assemblyPanel);
    setColor(renderer, 75, 80, 90);
    SDL_RenderRect(renderer, &assemblyPanel);

    if (components.empty()) {
        drawText(renderer, 520, 210, "No components installed.");
        drawText(renderer, 520, 235, "Select modules from the catalog.");
    } else {
        int y = 205;

        for (const auto& component : components) {
            drawText(renderer, 515, static_cast<float>(y), component.c_str());
            y += 25;

            if (y > 550) {
                drawText(renderer, 515, static_cast<float>(y), "...");
                break;
            }
        }
    }

    // Telemetry
    drawText(renderer, 880, 155, "SPACECRAFT TELEMETRY", 150, 155, 165);

    SDL_FRect telemetryPanel = {870, 180, 370, 400};
    setColor(renderer, 27, 30, 37);
    SDL_RenderFillRect(renderer, &telemetryPanel);
    setColor(renderer, 75, 80, 90);
    SDL_RenderRect(renderer, &telemetryPanel);

    drawText(renderer, 895, 210, "Total Mass:");
    drawText(renderer, 895, 235, (std::to_string(totalMass) + " kg").c_str());

    drawText(renderer, 895, 280, "Power Generation:");
    drawText(renderer, 895, 305, (std::to_string(powerGeneration) + " W").c_str());

    drawText(renderer, 895, 350, "Power Consumption:");
    drawText(renderer, 895, 375, (std::to_string(powerConsumption) + " W").c_str());

    drawText(renderer, 895, 420, "Fuel Capacity:");
    drawText(renderer, 895, 445, (std::to_string(fuelCapacity) + " units").c_str());

    drawText(renderer, 895, 500, "Installed Components:");
    drawText(renderer, 895, 525, std::to_string(components.size()).c_str());
}

void Game::renderPlanner() {
    drawText(renderer, 250, 90, "MISSION PLANNER", 247, 151, 40);
    drawText(renderer, 250, 125, "Choose a destination and validate your spacecraft.");

    drawText(renderer, 250, 165, "DESTINATION", 150, 155, 165);

    drawButton(renderer, 240, 190, 200, 45, "Moon", destination == "Moon");
    drawButton(renderer, 240, 245, 200, 45, "Mars", destination == "Mars");
    drawButton(renderer, 240, 300, 200, 45, "Earth Orbit",
               destination == "Earth Orbit");

    drawText(renderer, 520, 165, "MISSION CONFIGURATION", 150, 155, 165);

    SDL_FRect panel = {510, 190, 500, 250};
    setColor(renderer, 27, 30, 37);
    SDL_RenderFillRect(renderer, &panel);
    setColor(renderer, 75, 80, 90);
    SDL_RenderRect(renderer, &panel);

    drawText(renderer, 540, 220, ("Destination: " + destination).c_str());
    drawText(renderer, 540, 260, "Mission Type: Exploration");
    drawText(renderer, 540, 300, "Launch Vehicle: Chemical Propulsion");

    bool ready = isSpacecraftReady();

    drawText(renderer, 540, 350,
             ready ? "Spacecraft Status: READY" : "Spacecraft Status: NOT READY",
             ready ? 80 : 240, ready ? 210 : 90, ready ? 120 : 90);

    drawButton(renderer, 240, 390, 200, 45, "Start Mission",
               ready);

    drawText(renderer, 520, 470, "Readiness requirements:");
    drawText(renderer, 540, 500, "- Command module installed");
    drawText(renderer, 540, 525, "- Chemical engine installed");
    drawText(renderer, 540, 550, "- Fuel tank installed");
    drawText(renderer, 540, 575, "- Power generation >= consumption");
}

void Game::renderMissionControl() {
    drawText(renderer, 250, 90, "MISSION CONTROL", 247, 151, 40);
    drawText(renderer, 250, 125, "Monitor and advance your active mission.");

    bool ready = isSpacecraftReady();

    SDL_FRect panel = {240, 165, 700, 220};
    setColor(renderer, 27, 30, 37);
    SDL_RenderFillRect(renderer, &panel);
    setColor(renderer, 75, 80, 90);
    SDL_RenderRect(renderer, &panel);

    drawText(renderer, 270, 195, ("Destination: " + destination).c_str());
    drawText(renderer, 270, 235, "Mission Status:");

    if (!ready) {
        drawText(renderer, 270, 270, "SPACECRAFT NOT READY",
                 240, 90, 90);
        drawText(renderer, 270, 310,
                 "Return to the Designer and fix the readiness requirements.");
    } else {
        drawText(renderer, 270, 270, missionPhases[missionPhase],
                 80, 210, 120);

        drawText(renderer, 270, 310,
                 ("Mission Phase: " + std::to_string(missionPhase + 1) +
                  " / 6").c_str());
    }

    drawButton(renderer, 240, 400, 220, 50, "Advance Mission Phase",
               ready && missionPhase < 5);

    drawText(renderer, 500, 410, "MISSION TIMELINE", 150, 155, 165);

    for (int i = 0; i < 6; i++) {
        drawText(renderer, 520, static_cast<float>(445 + i * 35),
                 missionPhases[i],
                 i <= missionPhase && ready ? 80 : 130,
                 i <= missionPhase && ready ? 210 : 135,
                 i <= missionPhase && ready ? 120 : 145);
    }

    if (ready && missionPhase == 5) {
        drawText(renderer, 240, 650, "MISSION COMPLETED!",
                 80, 210, 120);
    }
}
