
#pragma once

#include "Destination.h"
#include "LaunchVehicle.h"

#include <cstddef>
#include <vector>

enum class MissionPhase { Planning, InProgress, Completed, Failed };
enum class FlightFailure { None, Fuel, Power, Radiation, MissedBurn, PayloadOff };
enum class BurnStatus { Coasting, Ready, Firing, Complete, WindowMissed };
enum class FlightHazard { None, RadiationAhead, Radiation, DebrisAhead, Debris };

struct FlightStatus {
    bool arraysDeployed = false;
    bool payloadPowered = false;
    bool engineFiring = false;
    bool protectiveMode = false;
    double radiationDose = 0;
    double arrayHealth = 100;
    double progress = 0;
    double burnFuelKg = 0;
    double charge = 100;
    FlightFailure failure = FlightFailure::None;
};

struct FlightConditions {
    double burnFuelRemaining;
    double powerWatts;
    double windowSeconds;
    bool sunlight;
    BurnStatus burn;
    FlightHazard hazard;
};

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

    const LaunchVehicle& launchVehicle() const;
    LaunchVehicleId launchVehicleId() const;
    bool selectLaunchVehicle(LaunchVehicleId id);
    double launchMassKg() const;
    double launchVehiclePayloadCapacityKg() const;
    bool launchVehicleCompatible() const;

    bool canBeginMission() const;
    bool beginMission();

    const std::vector<FlightStep>& flightPlan() const;
    std::size_t completedSteps() const { return stepsCompleted; }
    double remainingFuelKg() const { return fuelKg; }
    double loadedFuelKg() const { return loadedKg; }
    bool canSetFuelLoad() const;
    void adjustFuelLoad(double changeKg);
    void advanceMission();
    bool inFlight() const;
    const FlightStatus& flight() const { return flightStatus; }
    FlightConditions flightConditions() const;
    void startBurn();
    void toggleSolarArrays();
    void togglePayload();
    void toggleProtectiveMode();
    void updateFlight(double seconds);

    bool restoreMission(MissionPhase savedPhase, std::size_t steps, double loaded,
                        double remaining, const FlightStatus& savedFlight);
    void returnToPlanning();
    double maxFuelLoadKg() const;

private:
    Spacecraft craft;
    const DestinationProfile* selectedDestination = &destinationCatalog().front();
    LaunchVehicleId selectedLaunchVehicle = LaunchVehicleId::PSLV;
    MissionPhase phase = MissionPhase::Planning;
    std::size_t stepsCompleted = 0;
    double fuelKg = 0;
    double loadedKg = 0;
    FlightStatus flightStatus;
};
