#pragma once

#include "Component.h"

#include <functional>
#include <vector>

struct SpacecraftStats {
    double dryMassKg = 0;
    double powerGenerationW = 0;
    double powerConsumptionW = 0;
    double fuelCapacityKg = 0;

    double powerBalanceW() const { return powerGenerationW - powerConsumptionW; }
};

class Spacecraft {
public:
    using InstalledComponents = std::vector<std::reference_wrapper<const Component>>;

    bool addComponent(ComponentId id);
    bool removeComponent(ComponentId id);
    bool hasComponent(ComponentId id) const;
    const InstalledComponents& components() const;
    SpacecraftStats stats() const;
    std::vector<const char*> readinessIssues() const;
    bool isReady() const;

private:
    InstalledComponents installed;
};
