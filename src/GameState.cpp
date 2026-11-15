
#include "GameState.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace {
constexpr double burnWindowStart = 0.45;
constexpr double burnWindowEnd = 0.95;
constexpr double secondsPerLeg = 20;
constexpr double burnDuration = 4;
constexpr double fuelTolerance = 1e-6;

// Scripted hazards on the lunar and Mars transfer leg.
constexpr double radiationStart = 0.18;
constexpr double radiationEnd = 0.40;
constexpr double debrisStart = 0.72;
constexpr double debrisEnd = 0.92;
constexpr double warningLead = 0.15;

double secondsInZone(double from, double to, double start, double end)
{
    return std::max(0.0, std::min(to, end) - std::max(from, start)) * secondsPerLeg;
}
}

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

// --------------------------------------------------
// Launch vehicle selection and compatibility
// --------------------------------------------------

const LaunchVehicle& GameState::launchVehicle() const
{
    const auto* vehicle = findLaunchVehicle(selectedLaunchVehicle);

    // Fallback protects against an invalid vehicle ID.
    return vehicle ? *vehicle : launchVehicleCatalog().front();
}

LaunchVehicleId GameState::launchVehicleId() const
{
    return selectedLaunchVehicle;
}

bool GameState::selectLaunchVehicle(LaunchVehicleId id)
{
    if (phase == MissionPhase::InProgress) {
        return false;
    }

    if (!findLaunchVehicle(id)) {
        return false;
    }

    if (selectedLaunchVehicle == id) {
        return true;
    }

    selectedLaunchVehicle = id;

    if (phase != MissionPhase::Planning) {
        returnToPlanning();
    }

    return true;
}

double GameState::launchMassKg() const
{
    const auto stats = craft.stats();
    return stats.dryMassKg + loadedKg;
}

double GameState::launchVehiclePayloadCapacityKg() const
{
    return payloadCapacity(launchVehicle(), destination().id);
}

double GameState::maxFuelLoadKg() const
{
    const auto stats = craft.stats();

    const double remainingPayload =
        launchVehiclePayloadCapacityKg() - stats.dryMassKg;

    return std::clamp(
        remainingPayload,
        0.0,
        stats.fuelCapacityKg
    );
}

bool GameState::launchVehicleCompatible() const
{
    return isVehicleCompatible(
        launchVehicle(),
        destination().id,
        launchMassKg()
    );
}

// --------------------------------------------------
// Mission readiness
// --------------------------------------------------

bool GameState::canBeginMission() const
{
    const auto checks = missionChecks(destination(), craft);

    const bool spacecraftReady = std::all_of(
        checks.begin(), checks.end(),
        [](const MissionCheck& check) {
            return check.met;
        }
    );

    const double requiredFuel = std::accumulate(
        flightPlan().begin(), flightPlan().end(), 0.0,
        [](double total, const FlightStep& step) {
            return total + step.fuelKg;
        }
    );

    return destination().available &&
           spacecraftReady &&
           launchVehicleCompatible() &&
           maxFuelLoadKg() >= requiredFuel;
}

bool GameState::beginMission()
{
    if (!canBeginMission()) {
        return false;
    }

    if (phase != MissionPhase::InProgress) {
        returnToPlanning();
        loadedKg = 0;
        fuelKg = 0;
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
    if (!canSetFuelLoad()) {
        return;
    }

    loadedKg = std::clamp(
        loadedKg + changeKg,
        0.0,
        maxFuelLoadKg()
    );

    fuelKg = loadedKg;
}

void GameState::advanceMission()
{
    if (phase != MissionPhase::InProgress) {
        return;
    }

    const auto& steps = flightPlan();
    const auto& next = steps[stepsCompleted];

    if (fuelKg < next.fuelKg) {
        flightStatus.failure = FlightFailure::Fuel;
        phase = MissionPhase::Failed;
        return;
    }

    fuelKg -= next.fuelKg;
    ++stepsCompleted;

    if (stepsCompleted == steps.size()) {
        phase = MissionPhase::Completed;
    }
}

bool GameState::inFlight() const
{
    return phase == MissionPhase::InProgress && stepsCompleted > 0;
}

FlightConditions GameState::flightConditions() const
{
    const bool sunlight = stepsCompleted != 1 || flightStatus.progress < 0.55 ||
                          flightStatus.progress >= 0.82;

    double power = 0;

    for (const Component& part : craft.components()) {
        if (sunlight && flightStatus.arraysDeployed) {
            power += part.powerGenerationW * flightStatus.arrayHealth / 100;
        }

        if ((part.id == ComponentId::ChemicalEngine && !flightStatus.engineFiring) ||
            (part.id == ComponentId::SciencePayload && !flightStatus.payloadPowered)) {
            continue;
        }

        power -= part.powerConsumptionW;
    }

    const auto& plan = flightPlan();

    const double remaining = stepsCompleted < plan.size()
        ? std::max(0.0, plan[stepsCompleted].fuelKg - flightStatus.burnFuelKg)
        : 0;

    BurnStatus burn = BurnStatus::Coasting;

    if (remaining <= fuelTolerance) {
        burn = BurnStatus::Complete;
    } else if (flightStatus.progress >= burnWindowEnd) {
        burn = BurnStatus::WindowMissed;
    } else if (flightStatus.engineFiring) {
        burn = BurnStatus::Firing;
    } else if (flightStatus.progress >= burnWindowStart) {
        burn = BurnStatus::Ready;
    }

    const double nextBoundary = burn == BurnStatus::Coasting
        ? burnWindowStart
        : burnWindowEnd;

    FlightHazard hazard = FlightHazard::None;
    const double progress = flightStatus.progress;

    if (stepsCompleted == 2 && inFlight()) {
        if (progress >= radiationStart - warningLead && progress < radiationEnd) {
            hazard = progress < radiationStart
                ? FlightHazard::RadiationAhead
                : FlightHazard::Radiation;
        } else if (progress >= debrisStart - warningLead && progress < debrisEnd) {
            hazard = progress < debrisStart
                ? FlightHazard::DebrisAhead
                : FlightHazard::Debris;
        }
    }

    return {
        remaining,
        power,
        std::max(0.0, (nextBoundary - progress) * secondsPerLeg),
        sunlight,
        burn,
        hazard
    };
}

void GameState::startBurn()
{
    if (inFlight() && flightConditions().burn == BurnStatus::Ready) {
        flightStatus.engineFiring = true;
    }
}

void GameState::togglePayload()
{
    if (inFlight() && !flightStatus.protectiveMode &&
        craft.hasComponent(ComponentId::SciencePayload)) {
        flightStatus.payloadPowered = !flightStatus.payloadPowered;
    }
}

void GameState::toggleProtectiveMode()
{
    if (!inFlight()) {
        return;
    }

    flightStatus.protectiveMode = !flightStatus.protectiveMode;

    if (flightStatus.protectiveMode) {
        flightStatus.payloadPowered = false;
    }
}

void GameState::toggleSolarArrays()
{
    if (inFlight() && craft.hasComponent(ComponentId::SolarArray) &&
        (flightStatus.arraysDeployed || flightStatus.arrayHealth > 0)) {
        flightStatus.arraysDeployed = !flightStatus.arraysDeployed;
    }
}

void GameState::updateFlight(double seconds)
{
    if (!inFlight() || seconds <= 0) {
        return;
    }

    const auto conditions = flightConditions();

    const double end = conditions.burn == BurnStatus::Complete
        ? 1.0
        : burnWindowEnd;

    const double elapsed = std::clamp(
        (end - flightStatus.progress) * secondsPerLeg,
        0.0,
        seconds
    );

    const double burnRate = flightPlan()[stepsCompleted].fuelKg / burnDuration;

    const double firingTime = flightStatus.engineFiring
        ? std::min(elapsed, conditions.burnFuelRemaining / burnRate)
        : 0.0;

    const double requestedFuel = firingTime * burnRate;
    const double spent = std::min(fuelKg, requestedFuel);

    fuelKg -= spent;
    flightStatus.burnFuelKg += spent;

    const double previousProgress = flightStatus.progress;

    flightStatus.progress = std::min(
        end,
        previousProgress + elapsed / secondsPerLeg
    );

    // Scale the electrical load for a short mission.
    // The optional pack doubles the reserve.
    const double batteryCapacity = craft.hasComponent(ComponentId::BatteryPack)
        ? 96
        : 48;

    flightStatus.charge = std::clamp(
        flightStatus.charge + conditions.powerWatts * elapsed / batteryCapacity,
        0.0,
        100.0
    );

    if (stepsCompleted == 2) {
        const double radiation = secondsInZone(
            previousProgress,
            flightStatus.progress,
            radiationStart,
            radiationEnd
        );

        const double doseRate = flightStatus.protectiveMode ? 3.5 : 35.0;

        flightStatus.radiationDose = std::min(
            100.0,
            flightStatus.radiationDose + radiation * doseRate
        );

        if (flightStatus.arraysDeployed) {
            const double debris = secondsInZone(
                previousProgress,
                flightStatus.progress,
                debrisStart,
                debrisEnd
            );

            flightStatus.arrayHealth = std::max(
                0.0,
                flightStatus.arrayHealth - debris * 27.5
            );
        }
    }

    const bool burnComplete =
        flightPlan()[stepsCompleted].fuelKg - flightStatus.burnFuelKg <= fuelTolerance;

    if (burnComplete) {
        flightStatus.engineFiring = false;
    }

    if (requestedFuel > spent + fuelTolerance) {
        flightStatus.failure = FlightFailure::Fuel;
    } else if (flightStatus.charge <= 0) {
        flightStatus.failure = FlightFailure::Power;
    } else if (flightStatus.radiationDose >= 100) {
        flightStatus.failure = FlightFailure::Radiation;
    } else if (!burnComplete &&
               flightStatus.progress >= burnWindowEnd - 1e-9) {
        flightStatus.failure = FlightFailure::MissedBurn;
    } else if (flightStatus.progress >= 1.0 - 1e-9 &&
               stepsCompleted + 1 == flightPlan().size() &&
               destination().needsSciencePayload &&
               !flightStatus.payloadPowered) {
        flightStatus.failure = FlightFailure::PayloadOff;
    }

    if (flightStatus.failure != FlightFailure::None) {
        flightStatus.engineFiring = false;
        phase = MissionPhase::Failed;
    } else if (flightStatus.progress >= 1.0 - 1e-9) {
        flightStatus.progress = 0;
        flightStatus.burnFuelKg = 0;

        if (++stepsCompleted == flightPlan().size()) {
            phase = MissionPhase::Completed;
        }
    }
}

bool GameState::restoreMission(MissionPhase savedPhase, std::size_t steps,
                               double loaded, double remaining,
                               const FlightStatus& savedFlight)
{
    const auto end = flightPlan().size();

    if (savedPhase != MissionPhase::InProgress &&
        savedPhase != MissionPhase::Completed &&
        savedPhase != MissionPhase::Failed) {
        return false;
    }

    if (!canBeginMission() || steps > end ||
        (savedPhase == MissionPhase::Completed) != (steps == end) ||
        (savedPhase == MissionPhase::Failed) !=
            (savedFlight.failure != FlightFailure::None) ||
        !std::isfinite(loaded) || loaded < 0 ||
        loaded > maxFuelLoadKg() ||
        !std::isfinite(remaining) || remaining < 0 ||
        remaining > loaded ||
        !std::isfinite(savedFlight.progress) ||
        savedFlight.progress < 0 || savedFlight.progress > 1 ||
        (savedPhase != MissionPhase::Failed && savedFlight.progress >= 1) ||
        ((steps == 0 || steps == end) && savedFlight.progress != 0) ||
        !std::isfinite(savedFlight.charge) ||
        savedFlight.charge < 0 || savedFlight.charge > 100 ||
        !std::isfinite(savedFlight.radiationDose) ||
        savedFlight.radiationDose < 0 ||
        savedFlight.radiationDose > 100 ||
        !std::isfinite(savedFlight.arrayHealth) ||
        savedFlight.arrayHealth < 0 ||
        savedFlight.arrayHealth > 100 ||
        (savedFlight.protectiveMode && savedFlight.payloadPowered) ||
        !std::isfinite(savedFlight.burnFuelKg) ||
        savedFlight.burnFuelKg < 0 ||
        savedFlight.burnFuelKg >
            (steps > 0 && steps < end
                ? flightPlan()[steps].fuelKg + fuelTolerance
                : 0) ||
        (savedFlight.engineFiring &&
            (savedPhase != MissionPhase::InProgress ||
             steps == 0 ||
             savedFlight.progress < burnWindowStart ||
             savedFlight.progress >= burnWindowEnd)) ||
        (savedFlight.arraysDeployed &&
            !craft.hasComponent(ComponentId::SolarArray)) ||
        (savedFlight.payloadPowered &&
            !craft.hasComponent(ComponentId::SciencePayload))) {
        return false;
    }

    // Validate the saved launch mass against the selected vehicle.
    if (!isVehicleCompatible(
            launchVehicle(),
            destination().id,
            craft.stats().dryMassKg + loaded)) {
        return false;
    }

    phase = savedPhase;
    stepsCompleted = steps;
    loadedKg = loaded;
    fuelKg = remaining;
    flightStatus = savedFlight;

    return true;
}

void GameState::returnToPlanning()
{
    phase = MissionPhase::Planning;
    stepsCompleted = 0;
    fuelKg = 0;
    loadedKg = 0;
    flightStatus = {};
}
