#include "screens/MissionControl.h"
#include "SpaceArtwork.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <numeric>

namespace {
using namespace screenDrawing;

constexpr SDL_FRect flightPanel{32, 140, 700, 524};
constexpr SDL_FRect telemetryPanel{752, 140, 416, 524};
constexpr SDL_FRect backButton{32, 708, 240, 52};
constexpr SDL_FRect menuButton{292, 708, 220, 52};
constexpr SDL_FRect advanceButton{836, 708, 332, 52};
constexpr SDL_FRect lessFuel{72, 246, 154, 44};
constexpr SDL_FRect moreFuel{236, 246, 154, 44};
constexpr SDL_FRect flightView{56, 208, 652, 248};
constexpr SDL_FRect burnButton{56, 519, 306, 44};
constexpr SDL_FRect arraysButton{378, 519, 330, 44};
constexpr SDL_FRect payloadButton{56, 576, 306, 44};
constexpr SDL_FRect protectiveButton{378, 576, 330, 44};

constexpr std::array<SDL_FPoint, 5> transferRoute{{
    {237, 378}, {276, 333}, {354, 290}, {483, 264}, {594, 276}
}};
constexpr std::array<SDL_FPoint, 3> earthRoute{{
    transferRoute[0], transferRoute[1], {192, 278}
}};

template <std::size_t Size>
void drawFlightPath(SDL_Renderer* renderer, const std::array<SDL_FPoint, Size>& route,
                    float progress, float fall, bool engineFiring)
{
    constexpr float radiansToDegrees = 57.29578f;
    const auto segment = std::min(static_cast<std::size_t>(progress), Size - 2);
    const float fraction = progress - static_cast<float>(segment);
    const auto start = route[segment];
    const auto end = route[segment + 1];
    const float x = start.x + (end.x - start.x) * fraction + 14 * fall;
    const float y = start.y + (end.y - start.y) * fraction + flightView.h * fall * fall;
    for (std::size_t i = 1; i < Size; ++i) {
        setColor(renderer, i <= progress ? cyan : border);
        SDL_RenderLine(renderer, route[i - 1].x, route[i - 1].y, route[i].x, route[i].y);
    }
    setColor(renderer, cyan);
    SDL_RenderLine(renderer, start.x, start.y,
                   start.x + (end.x - start.x) * fraction,
                   start.y + (end.y - start.y) * fraction);

    const SDL_Rect clip{static_cast<int>(flightView.x + 1), static_cast<int>(flightView.y + 1),
                        static_cast<int>(flightView.w - 2), static_cast<int>(flightView.h - 2)};
    const SDL_FRect sprite{x - 35, y - 35, 70, 70};
    const float heading = std::atan2(end.x - start.x, start.y - end.y) * radiansToDegrees;
    if (auto* ship = spaceArtwork::texture(renderer, spaceArtwork::ship)) {
        SDL_SetRenderClipRect(renderer, &clip);
        if (engineFiring) {
            const float length = std::hypot(end.x - start.x, end.y - start.y);
            const float dx = (end.x - start.x) / length;
            const float dy = (end.y - start.y) / length;
            setColor(renderer, amber);
            SDL_RenderLine(renderer, x - dx * 20, y - dy * 20, x - dx * 52, y - dy * 52);
            setColor(renderer, cyan);
            SDL_RenderLine(renderer, x - dx * 20, y - dy * 20, x - dx * 39, y - dy * 39);
        }
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
    : state(gameState), flightProgress(static_cast<float>(gameState.completedSteps() +
                                                           gameState.flight().progress)),
      fallProgress(gameState.missionPhase() == MissionPhase::Failed ? 1.0f : 0.0f)
{
}

void MissionControl::handleEvents(const SDL_Event& event)
{
    const auto phase = state.missionPhase();
    const bool finished = phase == MissionPhase::Completed || phase == MissionPhase::Failed;
    const bool flying = state.inFlight();
    const bool prelaunch = state.canSetFuelLoad();
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        switch (event.key.key) {
        case SDLK_ESCAPE: requestScreenChange(ScreenType::Planner); break;
        case SDLK_RETURN: if (prelaunch) state.advanceMission(); break;
        case SDLK_LEFT: state.adjustFuelLoad(-100); break;
        case SDLK_RIGHT: state.adjustFuelLoad(100); break;
        case SDLK_B: state.startBurn(); break;
        case SDLK_S: state.toggleSolarArrays(); break;
        case SDLK_P: state.togglePayload(); break;
        case SDLK_R: state.toggleProtectiveMode(); break;
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
    } else if (prelaunch && SDL_PointInRectFloat(&click, &advanceButton)) {
        state.advanceMission();
    } else if (prelaunch && SDL_PointInRectFloat(&click, &lessFuel)) {
        state.adjustFuelLoad(-100);
    } else if (prelaunch && SDL_PointInRectFloat(&click, &moreFuel)) {
        state.adjustFuelLoad(100);
    } else if (flying) {
        if (SDL_PointInRectFloat(&click, &burnButton)) state.startBurn();
        if (SDL_PointInRectFloat(&click, &arraysButton)) state.toggleSolarArrays();
        if (SDL_PointInRectFloat(&click, &payloadButton)) state.togglePayload();
        if (SDL_PointInRectFloat(&click, &protectiveButton)) state.toggleProtectiveMode();
    }
}

void MissionControl::update(float deltaTime)
{
    state.updateFlight(std::min(deltaTime, 0.1f));
    if (state.missionPhase() == MissionPhase::Failed) {
        fallProgress = std::min(1.0f, fallProgress + deltaTime / 0.95f);
    } else {
        flightProgress = std::min(static_cast<float>(state.completedSteps() + state.flight().progress),
                                  flightProgress + deltaTime / 1.35f);
    }
}

void MissionControl::render(SDL_Renderer* renderer)
{
    const auto& plan = state.flightPlan();
    const auto phase = state.missionPhase();
    const auto completed = state.completedSteps();
    const bool active = phase == MissionPhase::InProgress;
    const bool flying = state.inFlight();
    const bool failed = phase == MissionPhase::Failed;
    const bool finished = failed || phase == MissionPhase::Completed;
    const auto stats = state.spacecraft().stats();
    const auto mouse = pointerPosition(renderer);

    spaceArtwork::drawBackdrop(renderer);
    drawText(renderer, 32, 28, "MISSION CONTROL", text, 4);
    drawText(renderer, 34, 94, state.destination().objective, muted, 1.5f);
    drawText(renderer, 880, 42, "DESTINATION", muted, 1.5f);
    drawText(renderer, 880, 67, state.destinationName(), cyan, 2.5f);

    drawBox(renderer, flightPanel, panel, border);
    drawBox(renderer, telemetryPanel, panel, border);
    drawText(renderer, 56, 164, "FLIGHT VIEW", cyan, 2.25f);
    drawText(renderer, 776, 164, "FLIGHT PLAN", cyan, 2.25f);
    drawText(renderer, 776, 398, "TELEMETRY", cyan, 2.25f);

    char detail[128];
    for (std::size_t i = 0; i < plan.size(); ++i) {
        const SDL_FRect row{776, 208 + 44 * static_cast<float>(i), 368, 38};
        const bool current = i == completed && (active || failed);
        drawBox(renderer, row, control, current ? (failed ? amber : cyan) : border);
        drawText(renderer, 788, row.y + 8, plan[i].name, text, 1.5f);
        std::snprintf(detail, sizeof(detail), "%.0f kg", plan[i].fuelKg);
        drawText(renderer, 959, row.y + 8, detail, muted, 1.375f);
        const char* status = i < completed ? "Complete" :
            current ? (failed ? "Failed" : flying ? "Flying" : "Ready") : "Pending";
        drawText(renderer, 1046, row.y + 8, status,
                 i < completed ? green : current ? (failed ? amber : cyan) : muted, 1.375f);
    }

    const double required = std::accumulate(plan.begin(), plan.end(), 0.0,
        [](double total, const FlightStep& step) { return total + step.fuelKg; });
    const auto& flight = state.flight();
    const auto conditions = state.flightConditions();
    const bool hazardAlert = conditions.hazard != FlightHazard::None;
    const bool launched = completed > 0;
    telemetry(renderer, 442, "Dry mass", stats.dryMassKg, "%.0f kg");
    telemetry(renderer, 474, "Power balance",
              launched ? conditions.powerWatts : stats.powerBalanceW(), "%+.0f W");
    telemetry(renderer, 506, launched ? "Battery charge" : "Fuel loaded",
              launched ? flight.charge : state.loadedFuelKg(), launched ? "%.0f%%" : "%.0f kg");
    telemetry(renderer, 538, "Fuel remaining", state.remainingFuelKg(), "%.0f kg");
    telemetry(renderer, 570, flying ? "Burn fuel needed" : "Reference fuel",
              flying ? conditions.burnFuelRemaining : required, "%.0f kg");

    const bool prelaunch = state.canSetFuelLoad();
    const auto destination = state.destination().id;
    const bool transferMission = destination != Destination::EarthOrbit;
    const auto target = transferMission ? destination : Destination::Moon;
    drawBox(renderer, flightView, background, hazardAlert ? amber : border);
    const SDL_FRect viewInterior{flightView.x + 1, flightView.y + 1,
                                 flightView.w - 2, flightView.h - 2};
    const char* sky = destination == Destination::Mars ? spaceArtwork::nebula :
        transferMission ? spaceArtwork::lunarTransfer : spaceArtwork::launchSky;
    spaceArtwork::drawSky(renderer, sky, viewInterior, 185);
    spaceArtwork::drawSprite(renderer, spaceArtwork::earth, {93, 302, 142, 142});
    spaceArtwork::drawSprite(renderer, spaceArtwork::planet(target), {568, 293, 118, 118},
                             transferMission ? 255 : 170);
    drawText(renderer, 75, 428, "Earth", text, 1.25f);
    drawText(renderer, 606, 416, target == Destination::Mars ? "Mars" : "Moon", text, 1.25f);
    if (transferMission) {
        drawFlightPath(renderer, transferRoute, flightProgress, fallProgress, flight.engineFiring);
    } else {
        drawFlightPath(renderer, earthRoute, flightProgress, fallProgress, flight.engineFiring);
    }
    if (prelaunch) {
        drawText(renderer, 72, 219, "Set fuel before launch", text, 1.5f);
        drawButton(renderer, lessFuel, mouse, "-100 kg", state.loadedFuelKg() > 0);
        drawButton(renderer, moreFuel, mouse, "+100 kg", state.loadedFuelKg() < stats.fuelCapacityKg);
    }
    const SDL_FRect track{776, 608, 368, 10};
    SDL_FRect progress = track;
    progress.w *= static_cast<float>(completed + flight.progress) / static_cast<float>(plan.size());
    setColor(renderer, border);
    SDL_RenderFillRect(renderer, &track);
    setColor(renderer, phase == MissionPhase::Completed ? green : cyan);
    if (completed > 0) {
        SDL_RenderFillRect(renderer, &progress);
    }
    std::snprintf(detail, sizeof(detail), "%zu / %zu phases complete", completed, plan.size());
    drawText(renderer, 776, 628, detail, muted, 1.25f);

    if (flying) {
        drawText(renderer, 56, 473, "FLIGHT CONTROLS", cyan, 2.25f);
        const bool ready = conditions.burn == BurnStatus::Ready;
        const char* burnLabel = "Burn window closed";
        switch (conditions.burn) {
        case BurnStatus::Coasting: burnLabel = "Awaiting burn window"; break;
        case BurnStatus::Ready: burnLabel = "Fire engine (B)"; break;
        case BurnStatus::Firing: burnLabel = "Engine firing"; break;
        case BurnStatus::Complete: burnLabel = "Burn complete"; break;
        case BurnStatus::WindowMissed: break;
        }
        drawButton(renderer, burnButton, mouse, burnLabel, ready, ready);
        drawButton(renderer, arraysButton, mouse,
                   flight.arraysDeployed ? "Stow arrays (S)" : "Deploy arrays (S)",
                   state.spacecraft().hasComponent(ComponentId::SolarArray) &&
                   (flight.arraysDeployed || flight.arrayHealth > 0));
        if (state.spacecraft().hasComponent(ComponentId::SciencePayload)) {
            drawButton(renderer, payloadButton, mouse,
                       flight.payloadPowered ? "Power off payload (P)" : "Power on payload (P)",
                       !flight.protectiveMode, flight.payloadPowered);
        }
        drawButton(renderer, protectiveButton, mouse,
                   flight.protectiveMode ? "Exit protective mode (R)" : "Protective mode (R)",
                   true, flight.protectiveMode);
        const char* powerStatus = "Deploy solar arrays (S) for power.";
        switch (conditions.hazard) {
        case FlightHazard::RadiationAhead:
        case FlightHazard::Radiation:
            powerStatus = flight.protectiveMode ? "Radiation protection active." :
                "Radiation alert! Enable protective mode (R).";
            break;
        case FlightHazard::DebrisAhead:
        case FlightHazard::Debris:
            powerStatus = flight.arraysDeployed ? "Asteroid debris! Stow solar arrays (S)." :
                "Arrays stowed. Keep them stowed until clear.";
            break;
        case FlightHazard::None:
            if (!conditions.sunlight) powerStatus = "Earth shadow. Running on battery.";
            else if (flight.arrayHealth <= 0) powerStatus = "Solar arrays destroyed. Running on battery.";
            else if (flight.protectiveMode) powerStatus = "Radiation clear. Exit protective mode (R).";
            else if (state.destination().needsSciencePayload && completed + 1 == plan.size() &&
                     !flight.payloadPowered && flight.arraysDeployed)
                powerStatus = "Power on the science payload (P) before Mars arrival.";
            else if (flight.arraysDeployed) powerStatus = "Solar arrays deployed.";
            break;
        }
        drawText(renderer, 56, 632, powerStatus,
                 hazardAlert || conditions.powerWatts < 0 ? amber : muted, 1.375f);
        std::snprintf(detail, sizeof(detail), "Arrays %.0f%%  Radiation %.0f%%",
                      flight.arrayHealth, flight.radiationDose);
        drawText(renderer, 776, 674, detail, flight.arrayHealth < 100 || flight.radiationDose > 0 ?
                 amber : muted, 1.375f);
        if (conditions.burn == BurnStatus::Coasting) {
            std::snprintf(detail, sizeof(detail), "Coasting. Burn window opens in %.0f s.", std::ceil(conditions.windowSeconds));
        } else if (ready || flight.engineFiring) {
            std::snprintf(detail, sizeof(detail), "%s. Window closes in %.0f s.",
                          ready ? "Fire engine (B). Burn takes 4 s" : "Maneuver underway",
                          std::ceil(conditions.windowSeconds));
        } else {
            std::snprintf(detail, sizeof(detail), "Maneuver complete. Coasting to the next phase.");
        }
        drawText(renderer, 32, 674, detail, ready ? amber : cyan, 1.5f);
    } else {
        drawText(renderer, 56, 484, "MISSION LOG", cyan, 2.25f);
        drawText(renderer, 56, 522, "Prelaunch checks passed. Spacecraft is ready.", muted, 1.375f);
        for (std::size_t i = 0; i < completed; ++i) {
            drawText(renderer, 56, 544 + 20 * static_cast<float>(i), plan[i].event, muted, 1.375f);
        }
        if (failed) {
            const char* reason = "Insufficient fuel to continue the flight.";
            if (flight.failure == FlightFailure::Power) reason = "Battery depleted. Spacecraft lost power.";
            if (flight.failure == FlightFailure::Radiation) reason = "Radiation disabled the spacecraft.";
            if (flight.failure == FlightFailure::MissedBurn) reason = "Burn window missed. The spacecraft missed its target.";
            if (flight.failure == FlightFailure::PayloadOff) reason = "Mars orbit reached. Science payload was off.";
            drawText(renderer, 56, 544 + 20 * static_cast<float>(completed), reason, amber, 1.375f);
        }
    }

    drawButton(renderer, backButton, mouse, "< Planner (Esc)", true);
    if (finished) {
        drawButton(renderer, menuButton, mouse, "Main menu (M)", true);
        drawText(renderer, 836, 722, failed ? "MISSION FAILED" : "MISSION COMPLETE",
                 failed ? amber : green, 2.25f);
    } else if (prelaunch) {
        drawButton(renderer, advanceButton, mouse, "Launch (Enter)", active, true);
    }
}
