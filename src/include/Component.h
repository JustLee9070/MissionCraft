#pragma once

#include <array>

enum class ComponentId {
    CommandModule,
    ChemicalEngine,
    FuelTank,
    SolarArray,
    BatteryPack,
    HighGainAntenna,
    SciencePayload
};

struct Component {
    ComponentId id;
    const char* name;
    double massKg;
    double powerGenerationW;
    double powerConsumptionW;
    double fuelCapacityKg;
};

const std::array<Component, 7>& componentCatalog();
