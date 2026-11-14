#pragma once

#include "Destination.h"

#include <cstddef>
#include <vector>

enum class MissionPhase { Planning, InProgress, Completed, Failed };

struct FlightStep {
    const char* name;
    const char* event;
    double fuelKg;
};

class GameState {
public:
    Spacecraft& spacecraft() { return craft; }
    const Spacecraft& spacecraft() const { return craft; }
    const DestinationProfile& destination() const;
    const char* destinationName() const;
    MissionPhase missionPhase() const;
    void selectDestination(Destination destination);
    bool canBeginMission() const;
    bool beginMission();
    const std::vector<FlightStep>& flightPlan() const;
    std::size_t completedSteps() const { return stepsCompleted; }
    double remainingFuelKg() const { return fuelKg; }
    double loadedFuelKg() const { return loadedKg; }
    bool canSetFuelLoad() const;
    void adjustFuelLoad(double changeKg);
    void advanceMission();
    void returnToPlanning();

private:
    Spacecraft craft;
    const DestinationProfile* selectedDestination = &destinationCatalog().front();
    MissionPhase phase = MissionPhase::Planning;
    std::size_t stepsCompleted = 0;
    double fuelKg = 0;
    double loadedKg = 0;
};
