
#pragma once

#include "Destination.h"

#include <array>
#include <cstddef>

enum class LaunchVehicleId {
    SSLV,
    PSLV,
    GSLV,
    Falcon9
};

struct LaunchVehicle {
    LaunchVehicleId id;
    const char* name;
    const char* imagePath;
    std::array<double, 3> payloadCapacityKg;
    double cost;
    double reliability;
    bool reusable;
};

const std::array<LaunchVehicle, 4>& launchVehicleCatalog();

const LaunchVehicle* findLaunchVehicle(LaunchVehicleId id);

double payloadCapacity(const LaunchVehicle& vehicle, Destination destination);

bool isVehicleCompatible(const LaunchVehicle& vehicle, Destination destination,
                         double payloadMassKg);
