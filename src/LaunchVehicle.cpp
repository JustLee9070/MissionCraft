
#include <cmath>
#include "LaunchVehicle.h"

const std::array<LaunchVehicle, 4>& launchVehicleCatalog()
{
    // Simplified gameplay capacities in kg: Earth orbit, Moon, Mars.
    static const std::array<LaunchVehicle, 4> catalog{{
        {LaunchVehicleId::SSLV, "SSLV", "assets/launch_vehicles/sslv.png",
         {2500, 0, 0}, 12'000'000, 0.96, false},

        {LaunchVehicleId::PSLV, "PSLV", "assets/launch_vehicles/pslv.png",
         {5000, 3000, 2000}, 35'000'000, 0.97, false},

        {LaunchVehicleId::GSLV, "GSLV", "assets/launch_vehicles/gslv.png",
         {8000, 5000, 0}, 75'000'000, 0.96, false},

        {LaunchVehicleId::Falcon9, "Falcon 9", "assets/launch_vehicles/falcon9.png",
         {15000, 8000, 4000}, 67'000'000, 0.99, true}
    }};

    return catalog;
}

const LaunchVehicle* findLaunchVehicle(LaunchVehicleId id)
{
    const auto& catalog = launchVehicleCatalog();

    for (const auto& vehicle : catalog) {
        if (vehicle.id == id) {
            return &vehicle;
        }
    }

    return nullptr;
}

double payloadCapacity(const LaunchVehicle& vehicle, Destination destination)
{
    const auto index = static_cast<std::size_t>(destination);
    return index < vehicle.payloadCapacityKg.size()
        ? vehicle.payloadCapacityKg[index]
        : 0;
}

bool isVehicleCompatible(const LaunchVehicle& vehicle, Destination destination,
                         double payloadMassKg)
{
    return std::isfinite(payloadMassKg) && payloadMassKg >= 0 &&
           payloadCapacity(vehicle, destination) > 0 &&
           payloadMassKg <= payloadCapacity(vehicle, destination);
}
