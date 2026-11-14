#include "screens/MissionControl.h"
#include "SpaceArtwork.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <numeric>

namespace {
using namespace screenDrawing;

constexpr SDL_FRect timelinePanel{32, 140, 700, 524};
constexpr SDL_FRect telemetryPanel{752, 140, 416, 524};
constexpr SDL_FRect backButton{32, 708, 240, 52};
constexpr SDL_FRect menuButton{292, 708, 220, 52};
constexpr SDL_FRect advanceButton{836, 708, 332, 52};
constexpr SDL_FRect lessFuel{776, 444, 174, 44};
constexpr SDL_FRect moreFuel{970, 444, 174, 44};
constexpr SDL_FRect flightView{776, 401, 368, 196};

constexpr std::array<SDL_FPoint, 5> transferRoute{{
    {876, 541}, {896, 509}, {934, 473}, {1006, 450}, {1065, 457}
}};
constexpr std::array<SDL_FPoint, 3> earthRoute{{
    transferRoute[0], transferRoute[1], {855, 460}
}};

template <std::size_t Size>
void drawFlightPath(SDL_Renderer* renderer, const std::array<SDL_FPoint, Size>& route,
                    float progress, float fall)
{
    constexpr float radiansToDegrees = 57.29578f;
    const auto segment = std::min(static_cast<std::size_t>(progress), Size - 2);
    const float fraction = progress - static_cast<float>(segment);
    const auto start = route[segment];
    const auto end = route[segment + 1];
    const float x = start.x + (end.x - start.x) * fraction + 14 * fall;
    const float y = start.y + (end.y - start.y) * fraction + 195 * fall * fall;
    for (std::size_t i = 1; i < Size; ++i) {
        setColor(renderer, i <= progress ? cyan : border);
        SDL_RenderLine(renderer, route[i - 1].x, route[i - 1].y, route[i].x, route[i].y);
    }

    const SDL_Rect clip{777, 402, 366, 194};
    const SDL_FRect sprite{x - 22, y - 22, 44, 44};
    const float heading = std::atan2(end.x - start.x, start.y - end.y) * radiansToDegrees;
    if (auto* ship = spaceArtwork::texture(renderer, spaceArtwork::ship)) {
        SDL_SetRenderClipRect(renderer, &clip);
        SDL_RenderTextureRotated(renderer, ship, nullptr, &sprite, heading + 155 * fall,
                                 nullptr, SDL_FLIP_NONE);
        SDL_SetRenderClipRect(renderer, nullptr);
    }
}

void telemetry(SDL_Renderer* renderer, float y, const char* label, double value, const char* format)
{
    char buffer[48];
    std::snprintf(buffer, sizeof(buffer), format, value);
    drawText(renderer, 776, y, label, muted, 1.5f);
    drawText(renderer, 1000, y, buffer, text, 1.5f);
}
}

MissionControl::MissionControl(GameState& gameState)
    : state(gameState), flightProgress(static_cast<float>(gameState.completedSteps())),
      fallProgress(gameState.missionPhase() == MissionPhase::Failed ? 1.0f : 0.0f)
{
}

void MissionControl::handleEvents(const SDL_Event& event)
{
    const auto phase = state.missionPhase();
    const bool finished = phase == MissionPhase::Completed || phase == MissionPhase::Failed;
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        switch (event.key.key) {
        case SDLK_ESCAPE: requestScreenChange(ScreenType::Planner); break;
        case SDLK_RETURN: state.advanceMission(); break;
        case SDLK_LEFT: state.adjustFuelLoad(-100); break;
        case SDLK_RIGHT: state.adjustFuelLoad(100); break;
        case SDLK_M:
            if (finished) {
                requestScreenChange(ScreenType::MainMenu);
            }
            break;
        }
        return;
    }

    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN || event.button.button != SDL_BUTTON_LEFT) {
        return;
    }
    const SDL_FPoint click{event.button.x, event.button.y};
    if (SDL_PointInRectFloat(&click, &backButton)) {
        requestScreenChange(ScreenType::Planner);
    } else if (finished && SDL_PointInRectFloat(&click, &menuButton)) {
        requestScreenChange(ScreenType::MainMenu);
    } else if (SDL_PointInRectFloat(&click, &advanceButton)) {
        state.advanceMission();
    } else if (SDL_PointInRectFloat(&click, &lessFuel)) {
        state.adjustFuelLoad(-100);
    } else if (SDL_PointInRectFloat(&click, &moreFuel)) {
        state.adjustFuelLoad(100);
    }
}

void MissionControl::update(float deltaTime)
{
    if (state.missionPhase() == MissionPhase::Failed) {
        fallProgress = std::min(1.0f, fallProgress + deltaTime / 0.95f);
    } else {
        flightProgress = std::min(static_cast<float>(state.completedSteps()),
                                  flightProgress + deltaTime / 0.85f);
    }
}

void MissionControl::render(SDL_Renderer* renderer)
{
    const auto& plan = state.flightPlan();
    const auto phase = state.missionPhase();
    const auto completed = state.completedSteps();
    const bool active = phase == MissionPhase::InProgress;
    const bool failed = phase == MissionPhase::Failed;
    const bool finished = failed || phase == MissionPhase::Completed;
    const auto stats = state.spacecraft().stats();
    const auto mouse = pointerPosition(renderer);

    spaceArtwork::drawBackdrop(renderer);
    drawText(renderer, 32, 28, "MISSION CONTROL", text, 4);
    drawText(renderer, 34, 94, state.destination().objective, muted, 1.5f);
    drawText(renderer, 880, 42, "DESTINATION", muted, 1.5f);
    drawText(renderer, 880, 67, state.destinationName(), cyan, 2.5f);

    drawBox(renderer, timelinePanel, panel, border);
    drawBox(renderer, telemetryPanel, panel, border);
    drawText(renderer, 56, 164, "FLIGHT PLAN", cyan, 2.25f);
    drawText(renderer, 776, 164, "TELEMETRY", cyan, 2.25f);

    char detail[128];
    for (std::size_t i = 0; i < plan.size(); ++i) {
        const SDL_FRect row{56, 222 + 58 * static_cast<float>(i), 652, 48};
        const bool current = i == completed && (active || failed);
        drawBox(renderer, row, control, current ? (failed ? amber : cyan) : border);
        drawText(renderer, 72, row.y + 10, plan[i].name, text, 1.75f);
        std::snprintf(detail, sizeof(detail), "%.0f kg", plan[i].fuelKg);
        drawText(renderer, 434, row.y + 13, detail, muted, 1.5f);
        const char* status = i < completed ? "Complete" :
            current ? (failed ? "Failed" : "Next") : "Pending";
        drawText(renderer, 588, row.y + 13, status,
                 i < completed ? green : current ? (failed ? amber : cyan) : muted, 1.5f);
    }

    const double required = std::accumulate(plan.begin(), plan.end(), 0.0,
        [](double total, const FlightStep& step) { return total + step.fuelKg; });
    telemetry(renderer, 222, "Dry mass", stats.dryMassKg, "%.0f kg");
    telemetry(renderer, 256, "Power balance", stats.powerBalanceW(), "%+.0f W");
    telemetry(renderer, 290, "Fuel loaded", state.loadedFuelKg(), "%.0f kg");
    telemetry(renderer, 324, "Fuel remaining", state.remainingFuelKg(), "%.0f kg");
    telemetry(renderer, 358, "Fuel required", required, "%.0f kg");

    const bool prelaunch = state.canSetFuelLoad();
    const auto destination = state.destination().id;
    const bool transferMission = destination != Destination::EarthOrbit;
    const auto target = transferMission ? destination : Destination::Moon;
    drawBox(renderer, flightView, background, border);
    const SDL_FRect viewInterior{flightView.x + 1, flightView.y + 1,
                                 flightView.w - 2, flightView.h - 2};
    const char* sky = destination == Destination::Mars ? spaceArtwork::nebula :
        transferMission ? spaceArtwork::lunarTransfer : spaceArtwork::launchSky;
    spaceArtwork::drawSky(renderer, sky, viewInterior, 185);
    spaceArtwork::drawSprite(renderer, spaceArtwork::earth, {791, 485, 106, 106});
    spaceArtwork::drawSprite(renderer, spaceArtwork::planet(target), {1049, 473, 79, 79},
                             transferMission ? 255 : 170);
    drawText(renderer, 804, 570, "Earth", text, 1.125f);
    drawText(renderer, 1068, 558, target == Destination::Mars ? "Mars" : "Moon", text, 1.125f);
    if (transferMission) {
        drawFlightPath(renderer, transferRoute, flightProgress, fallProgress);
    } else {
        drawFlightPath(renderer, earthRoute, flightProgress, fallProgress);
    }
    if (prelaunch) {
        drawText(renderer, 790, 411, "Set fuel before launch", text, 1.5f);
        drawButton(renderer, lessFuel, mouse, "-100 kg", state.loadedFuelKg() > 0);
        drawButton(renderer, moreFuel, mouse, "+100 kg", state.loadedFuelKg() < stats.fuelCapacityKg);
    }
    const SDL_FRect track{776, 608, 368, 10};
    SDL_FRect progress = track;
    progress.w *= static_cast<float>(completed) / static_cast<float>(plan.size());
    setColor(renderer, border);
    SDL_RenderFillRect(renderer, &track);
    setColor(renderer, phase == MissionPhase::Completed ? green : cyan);
    if (completed > 0) {
        SDL_RenderFillRect(renderer, &progress);
    }
    std::snprintf(detail, sizeof(detail), "%zu / %zu phases complete", completed, plan.size());
    drawText(renderer, 776, 628, detail, muted, 1.25f);

    drawText(renderer, 56, 484, "MISSION LOG", cyan, 2.25f);
    drawText(renderer, 56, 522, "Prelaunch checks passed. Spacecraft is ready.", muted, 1.375f);
    for (std::size_t i = 0; i < completed; ++i) {
        drawText(renderer, 56, 544 + 20 * static_cast<float>(i), plan[i].event, muted, 1.375f);
    }
    if (failed) {
        std::snprintf(detail, sizeof(detail), "%s needs %.0f kg. Only %.0f kg remains.",
                      plan[completed].name, plan[completed].fuelKg, state.remainingFuelKg());
        drawText(renderer, 56, 544 + 20 * static_cast<float>(completed), detail, amber, 1.375f);
    }

    drawButton(renderer, backButton, mouse, "< Planner (Esc)", true);
    if (finished) {
        drawButton(renderer, menuButton, mouse, "Main menu (M)", true);
        drawText(renderer, 836, 722, failed ? "MISSION FAILED" : "MISSION COMPLETE",
                 failed ? amber : green, 2.25f);
    } else {
        drawButton(renderer, advanceButton, mouse, completed == 0 ? "Launch (Enter)" :
                   "Advance phase (Enter)", active, true);
    }
}
