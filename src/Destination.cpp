#include "Destination.h"

const std::array<DestinationProfile, 3>& destinationCatalog()
{
    // Fuel thresholds are gameplay values.
    static constexpr std::array<DestinationProfile, 3> catalog{{
        {Destination::EarthOrbit, "Earth Orbit", "Easy", "Place the spacecraft in Earth orbit.",
         500, false, true},
        {Destination::Moon, "Moon", "Medium", "Reach lunar orbit and establish a radio link.",
         1000, true, true},
        {Destination::Mars, "Mars", "Advanced", "Reach Mars orbit and activate the science payload.",
         1000, true, true, true}
    }};
    return catalog;
}

std::array<MissionCheck, 6> missionChecks(const DestinationProfile& destination,
                                         const Spacecraft& craft)
{
    const auto totals = craft.stats();
    return {{
        {"Command module", craft.hasComponent(ComponentId::CommandModule)},
        {"Propulsion system", craft.hasComponent(ComponentId::ChemicalEngine)},
        {"Power balance", totals.powerBalanceW() >= 0},
        {"Fuel capacity", totals.fuelCapacityKg >= destination.minimumFuelKg},
        {"High-gain antenna", !destination.needsHighGainAntenna ||
                              craft.hasComponent(ComponentId::HighGainAntenna),
                              destination.needsHighGainAntenna},
        {"Science payload", !destination.needsSciencePayload ||
                            craft.hasComponent(ComponentId::SciencePayload),
                            destination.needsSciencePayload}
    }};
}
