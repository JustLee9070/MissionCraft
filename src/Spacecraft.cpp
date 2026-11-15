
#include "Spacecraft.h"

#include <algorithm>

bool Spacecraft::addComponent(ComponentId id)
{
    const auto matches = [id](const Component& part) {
        return part.id == id;
    };

    const auto& catalog = componentCatalog();
    const auto component = std::find_if(catalog.begin(), catalog.end(), matches);

    if (component == catalog.end() || hasComponent(id)) {
        return false;
    }

    installed.emplace_back(*component);
    return true;
}

bool Spacecraft::removeComponent(ComponentId id)
{
    const auto matches = [id](const Component& part) {
        return part.id == id;
    };

    const auto component = std::find_if(installed.begin(), installed.end(), matches);

    if (component == installed.end()) {
        return false;
    }

    installed.erase(component);
    return true;
}

const Spacecraft::InstalledComponents& Spacecraft::components() const
{
    return installed;
}

bool Spacecraft::hasComponent(ComponentId id) const
{
    return std::any_of(installed.begin(), installed.end(),
        [id](const Component& part) {
            return part.id == id;
        });
}

void Spacecraft::setConfiguration(SpacecraftConfiguration configuration)
{
    selectedConfiguration = configuration;
}

SpacecraftConfiguration Spacecraft::configuration() const
{
    return selectedConfiguration;
}

SpacecraftStats Spacecraft::stats() const
{
    SpacecraftStats totals;

    for (const Component& part : installed) {
        double mass = part.massKg;

        if (selectedConfiguration == SpacecraftConfiguration::Lightweight) {
            switch (part.id) {
            case ComponentId::CommandModule:
                mass = 150;
                break;
            case ComponentId::ChemicalEngine:
                mass = 100;
                break;
            case ComponentId::FuelTank:
                mass = 50;
                break;
            default:
                break;
            }
        }

        totals.dryMassKg += mass;
        totals.powerGenerationW += part.powerGenerationW;
        totals.powerConsumptionW += part.powerConsumptionW;

        if (part.id == ComponentId::FuelTank &&
            selectedConfiguration == SpacecraftConfiguration::Lightweight) {
            totals.fuelCapacityKg += 1000;
        } else {
            totals.fuelCapacityKg += part.fuelCapacityKg;
        }
    }

    return totals;
}

std::vector<const char*> Spacecraft::readinessIssues() const
{
    const auto totals = stats();
    std::vector<const char*> issues;

    if (!hasComponent(ComponentId::CommandModule)) {
        issues.push_back("Missing command module.");
    }

    if (!hasComponent(ComponentId::ChemicalEngine)) {
        issues.push_back("No propulsion system installed.");
    }

    if (totals.fuelCapacityKg <= 0) {
        issues.push_back("No fuel capacity.");
    }

    if (totals.powerBalanceW() < 0) {
        issues.push_back("Insufficient power generation.");
    }

    return issues;
}

bool Spacecraft::isReady() const
{
    return readinessIssues().empty();
}
