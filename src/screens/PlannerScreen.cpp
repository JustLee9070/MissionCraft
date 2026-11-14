#include "screens/PlannerScreen.h"
#include "SpaceArtwork.h"

#include <cstdio>

namespace {
using namespace screenDrawing;

constexpr SDL_FRect destinationsPanel{32, 140, 356, 534};
constexpr SDL_FRect requirementsPanel{408, 140, 760, 534};
constexpr SDL_FRect backButton{32, 708, 296, 52};
constexpr SDL_FRect startButton{850, 708, 318, 52};

SDL_FRect destinationRow(std::size_t index)
{
    return {48, 224 + 116 * static_cast<float>(index), 324, 92};
}
}

PlannerScreen::PlannerScreen(GameState& gameState)
    : state(gameState)
{
}

void PlannerScreen::startMission()
{
    if (state.beginMission()) {
        requestScreenChange(ScreenType::MissionControl);
    }
}

void PlannerScreen::handleEvents(const SDL_Event& event)
{
    const auto& catalog = destinationCatalog();
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        const auto key = event.key.key;
        if (key == SDLK_ESCAPE) {
            requestScreenChange(ScreenType::Designer);
        } else if (key == SDLK_RETURN) {
            startMission();
        } else if (key >= SDLK_1 && key < SDLK_1 + catalog.size()) {
            state.selectDestination(catalog[key - SDLK_1].id);
        }
        return;
    }

    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT) {
        return;
    }
    const SDL_FPoint click{event.button.x, event.button.y};
    if (SDL_PointInRectFloat(&click, &backButton)) {
        requestScreenChange(ScreenType::Designer);
        return;
    }
    if (SDL_PointInRectFloat(&click, &startButton)) {
        startMission();
        return;
    }
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        const auto row = destinationRow(i);
        if (SDL_PointInRectFloat(&click, &row)) {
            state.selectDestination(catalog[i].id);
            return;
        }
    }
}

void PlannerScreen::update(float deltaTime)
{
    (void)deltaTime;
}

void PlannerScreen::render(SDL_Renderer* renderer)
{
    const auto& catalog = destinationCatalog();
    const auto& destination = state.destination();
    const auto& craft = state.spacecraft();
    const auto checks = missionChecks(destination, craft);
    const bool canStart = state.canBeginMission();
    const auto mouse = pointerPosition(renderer);

    spaceArtwork::drawBackdrop(renderer);
    drawText(renderer, 32, 32, "MISSION PLANNER", text, 4);
    drawText(renderer, 32, 88, "Choose a destination and review the flight requirements.", muted);
    drawBox(renderer, destinationsPanel, panel, border);
    drawBox(renderer, requirementsPanel, panel, border);
    drawText(renderer, 48, 164, "DESTINATIONS", cyan, 2.25f);
    drawText(renderer, 48, 194, "Click or press 1-3 to choose", muted, 1.5f);
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        const auto row = destinationRow(i);
        const auto& entry = catalog[i];
        const bool selected = destination.id == entry.id;
        const bool hovered = entry.available && SDL_PointInRectFloat(&mouse, &row);
        drawBox(renderer, row, entry.available ? control : background, selected || hovered ? cyan : border);
        spaceArtwork::drawSprite(renderer, spaceArtwork::planet(entry.id),
                                 {row.x + 14, row.y + 13, 66, 66}, entry.available ? 255 : 185);
        drawText(renderer, row.x + 94, row.y + 14, entry.name, entry.available ? text : muted, 2.0f);
        drawText(renderer, row.x + 94, row.y + 54, entry.difficulty, muted, 1.5f);
        if (selected || !entry.available) {
            drawText(renderer, row.x + 220, row.y + 54, selected ? "Selected" : "Locked",
                     selected ? cyan : amber, 1.5f);
        }
    }

    drawText(renderer, 432, 164, destination.name, text, 3);
    drawText(renderer, 432, 210, destination.objective, muted, 1.5f);
    spaceArtwork::drawSprite(renderer, spaceArtwork::planet(destination.id), {1036, 153, 99, 99});
    drawText(renderer, 432, 270, "FLIGHT REQUIREMENTS", cyan, 2);
    drawText(renderer, 1000, 270, "STATUS", cyan, 2);

    const auto stats = craft.stats();
    for (std::size_t i = 0; i < checks.size(); ++i) {
        const auto& check = checks[i];
        const SDL_FRect row{432, 308 + 47 * static_cast<float>(i), 712, 42};
        drawBox(renderer, row, control, border);
        drawText(renderer, row.x + 14, row.y + 10, check.label);
        drawText(renderer, 1000, row.y + 11,
                 !check.required ? "Optional" : check.met ? "Met" : "Unmet",
                 !check.required ? muted : check.met ? green : amber, 1.75f);
    }
    char detail[64];
    std::snprintf(detail, sizeof(detail), "%+.0f W / min 0 W", stats.powerBalanceW());
    drawText(renderer, 740, 413, detail, muted, 1.5f);
    std::snprintf(detail, sizeof(detail), "%.0f / %.0f kg min", stats.fuelCapacityKg,
                  destination.minimumFuelKg);
    drawText(renderer, 740, 460, detail, muted, 1.5f);
    drawText(renderer, 432, 614, canStart ? "Ready for Mission Control" :
             "Return to Designer to meet the remaining requirements.", canStart ? green : amber, 1.5f);

    drawButton(renderer, backButton, mouse, "< Designer (Esc)", true);
    const auto phase = state.missionPhase();
    const char* action = phase == MissionPhase::InProgress ? "Resume mission (Enter)" :
        phase == MissionPhase::Planning ? "Start mission (Enter)" : "Fly again (Enter)";
    drawButton(renderer, startButton, mouse, action, canStart, true);
}
