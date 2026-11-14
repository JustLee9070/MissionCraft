#include "GameState.h"

#include <algorithm>

const DestinationProfile& GameState::destination() const
{
    return *selectedDestination;
}

const char* GameState::destinationName() const
{
    return destination().name;
}

MissionPhase GameState::missionPhase() const
{
    return phase;
}

void GameState::selectDestination(Destination destination)
{
    const auto& catalog = destinationCatalog();
    const auto match = std::find_if(catalog.begin(), catalog.end(),
        [destination](const DestinationProfile& candidate) {
            return candidate.id == destination && candidate.available;
        });
    if (match != catalog.end() && &*match != selectedDestination) {
        selectedDestination = &*match;
        returnToPlanning();
    }
}

bool GameState::canBeginMission() const
{
    const auto checks = missionChecks(destination(), craft);
    return destination().available && std::all_of(checks.begin(), checks.end(),
        [](const MissionCheck& check) { return check.met; });
}

bool GameState::beginMission()
{
    if (!canBeginMission()) {
        return false;
    }
    if (phase != MissionPhase::InProgress) {
        stepsCompleted = 0;
        loadedKg = craft.stats().fuelCapacityKg;
        fuelKg = loadedKg;
        phase = MissionPhase::InProgress;
    }
    return true;
}

const std::vector<FlightStep>& GameState::flightPlan() const
{
    // Simplified propellant costs for the game.
    static const std::vector<FlightStep> earth{
        {"Launch", "Launch successful. Ascent complete.", 300},
        {"Earth orbit", "Earth orbit established. Objective achieved.", 200}
    };
    static const std::vector<FlightStep> moon{
        earth[0],
        {"Earth orbit", "Earth parking orbit established.", 200},
        {"Lunar transfer", "Transfer burn complete. Approaching the Moon.", 350},
        {"Lunar orbit", "Lunar orbit established. Radio link confirmed.", 150}
    };
    static const std::vector<FlightStep> mars{
        moon[0],
        moon[1],
        {"Mars transfer", "Transfer burn complete. Approaching Mars.", 300},
        {"Mars orbit", "Mars orbit established. Science payload active.", 200}
    };
    switch (destination().id) {
    case Destination::Moon: return moon;
    case Destination::Mars: return mars;
    default: return earth;
    }
}

bool GameState::canSetFuelLoad() const
{
    return phase == MissionPhase::InProgress && stepsCompleted == 0;
}

void GameState::adjustFuelLoad(double changeKg)
{
    if (canSetFuelLoad()) {
        loadedKg = std::clamp(loadedKg + changeKg, 0.0, craft.stats().fuelCapacityKg);
        fuelKg = loadedKg;
    }
}

void GameState::advanceMission()
{
    if (phase != MissionPhase::InProgress) {
        return;
    }
    const auto& steps = flightPlan();
    const auto& next = steps[stepsCompleted];
    if (fuelKg < next.fuelKg) {
        phase = MissionPhase::Failed;
        return;
    }
    fuelKg -= next.fuelKg;
    ++stepsCompleted;
    if (stepsCompleted == steps.size()) {
        phase = MissionPhase::Completed;
    }
}

void GameState::returnToPlanning()
{
    phase = MissionPhase::Planning;
    stepsCompleted = 0;
    fuelKg = 0;
    loadedKg = 0;
}
