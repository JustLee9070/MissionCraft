#pragma once

#include "Spacecraft.h"

#include <array>

enum class Destination { EarthOrbit, Moon, Mars };

struct DestinationProfile {
    Destination id;
    const char* name;
    const char* difficulty;
    const char* objective;
    double minimumFuelKg;
    bool needsHighGainAntenna;
    bool available;
    bool needsSciencePayload = false;
};

struct MissionCheck {
    const char* label;
    bool met;
    bool required = true;
};

const std::array<DestinationProfile, 3>& destinationCatalog();
std::array<MissionCheck, 6> missionChecks(const DestinationProfile& destination,
                                         const Spacecraft& craft);
