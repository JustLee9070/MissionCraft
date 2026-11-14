#include "Component.h"

const std::array<Component, 7>& componentCatalog()
{
    // Gameplay values for the desktop prototype.
    static constexpr std::array<Component, 7> catalog{{
        {ComponentId::CommandModule, "Command Module", 1200, 0, 150, 0},
        {ComponentId::ChemicalEngine, "Chemical Engine", 250, 0, 30, 0},
        {ComponentId::FuelTank, "Fuel Tank", 180, 0, 0, 1000},
        {ComponentId::SolarArray, "Solar Array", 200, 300, 0, 0},
        {ComponentId::BatteryPack, "Battery Pack", 100, 0, 0, 0},
        {ComponentId::HighGainAntenna, "High-Gain Antenna", 80, 0, 45, 0},
        {ComponentId::SciencePayload, "Science Payload", 160, 0, 75, 0}
    }};
    return catalog;
}
