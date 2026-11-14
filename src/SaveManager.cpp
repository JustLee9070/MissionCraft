#include "SaveManager.h"

#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <random>
#include <utility>

namespace {
using nlohmann::json;
constexpr int saveVersion = 4;

bool readFlightStatus(const json& data, FlightStatus& flight, int version)
{
    const bool legacy = version == 2;
    const auto& failure = data.at("failure");
    if (!failure.is_number_integer() || failure < 0 ||
        failure > static_cast<int>(legacy ? FlightFailure::Radiation : FlightFailure::PayloadOff) ||
        !data.at("arraysDeployed").is_boolean() || !data.at("progress").is_number() ||
        !data.at("charge").is_number()) {
        return false;
    }
    flight.failure = static_cast<FlightFailure>(failure.get<int>());
    flight.arraysDeployed = data.at("arraysDeployed").get<bool>();
    flight.progress = data.at("progress").get<double>();
    flight.charge = data.at("charge").get<double>();
    if (!legacy) {
        if (!data.at("payloadPowered").is_boolean() || !data.at("engineFiring").is_boolean() ||
            !data.at("burnFuelKg").is_number()) {
            return false;
        }
        flight.payloadPowered = data.at("payloadPowered").get<bool>();
        flight.engineFiring = data.at("engineFiring").get<bool>();
        flight.burnFuelKg = data.at("burnFuelKg").get<double>();
    }
    if (version >= 4) {
        if (!data.at("protectiveMode").is_boolean() || !data.at("radiationDose").is_number() ||
            !data.at("arrayHealth").is_number()) {
            return false;
        }
        flight.protectiveMode = data.at("protectiveMode").get<bool>();
        flight.radiationDose = data.at("radiationDose").get<double>();
        flight.arrayHealth = data.at("arrayHealth").get<double>();
    }
    return true;
}

const char* screenKey(ScreenType screen)
{
    switch (screen) {
    case ScreenType::Designer: return "designer";
    case ScreenType::Planner: return "planner";
    case ScreenType::MissionControl: return "mission-control";
    default: return nullptr;
    }
}

std::optional<ScreenType> savedScreen(const json& key)
{
    for (auto screen : {ScreenType::Designer, ScreenType::Planner, ScreenType::MissionControl}) {
        if (key == screenKey(screen)) {
            return screen;
        }
    }
    return std::nullopt;
}

// A private staging directory keeps simultaneous writers from sharing a temporary file.
bool replaceSave(const std::filesystem::path& path, const json& document)
{
    std::error_code error;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            return false;
        }
    }
    auto staging = path;
    staging += ".tmp-" + std::to_string(std::random_device{}());
    if (!std::filesystem::create_directory(staging, error)) {
        return false;
    }
    const auto temporary = staging / "save.json";
    std::ofstream output(temporary);
    output << document.dump(2) << '\n';
    output.close();
    bool saved = false;
    if (output) {
        std::filesystem::rename(temporary, path, error);
        saved = !error;
    }
    std::filesystem::remove(temporary, error);
    std::filesystem::remove(staging, error);
    return saved;
}
}

SaveManager::SaveManager(std::filesystem::path path)
    : path(std::move(path))
{
}

bool SaveManager::hasSave() const
{
    std::error_code error;
    return std::filesystem::is_regular_file(path, error);
}

bool SaveManager::saveGame(const GameState& state, ScreenType screen) const
{
    const auto* key = screenKey(screen);
    if (!key) {
        return false;
    }
    auto components = json::array();
    for (const Component& component : state.spacecraft().components()) {
        components.push_back(static_cast<int>(component.id));
    }
    const auto& flight = state.flight();
    const json document{
        {"version", saveVersion},
        {"screen", key},
        {"components", std::move(components)},
        {"destination", static_cast<int>(state.destination().id)},
        {"phase", static_cast<int>(state.missionPhase())},
        {"completedSteps", state.completedSteps()},
        {"loadedFuelKg", state.loadedFuelKg()},
        {"remainingFuelKg", state.remainingFuelKg()},
        {"flight", {
            {"arraysDeployed", flight.arraysDeployed}, {"progress", flight.progress},
            {"charge", flight.charge}, {"burnFuelKg", flight.burnFuelKg},
            {"payloadPowered", flight.payloadPowered}, {"engineFiring", flight.engineFiring},
            {"protectiveMode", flight.protectiveMode}, {"radiationDose", flight.radiationDose},
            {"arrayHealth", flight.arrayHealth},
            {"failure", static_cast<int>(flight.failure)}
        }}
    };
    return replaceSave(path, document);
}

std::optional<SavedGame> SaveManager::loadGame() const
{
    std::ifstream input(path);
    if (!input) {
        return std::nullopt;
    }
    const auto document = json::parse(input, nullptr, false);
    try {
        const auto& version = document.at("version");
        if (!version.is_number_integer() || version < 1 || version > saveVersion) {
            return std::nullopt;
        }
        const auto screen = savedScreen(document.at("screen"));
        const auto& parts = document.at("components");
        const auto& destination = document.at("destination");
        const auto& phase = document.at("phase");
        const auto& steps = document.at("completedSteps");
        const auto& fuel = document.at("loadedFuelKg");
        if (!screen || !parts.is_array() || parts.size() > componentCatalog().size() ||
            !destination.is_number_integer() || !phase.is_number_integer() ||
            !steps.is_number_integer() || !fuel.is_number()) {
            return std::nullopt;
        }

        GameState state;
        for (const auto& part : parts) {
            if (!part.is_number_integer() || part.get<double>() < 0 ||
                part.get<double>() > static_cast<int>(ComponentId::SciencePayload) ||
                !state.spacecraft().addComponent(static_cast<ComponentId>(part.get<int>()))) {
                return std::nullopt;
            }
        }
        bool destinationFound = false;
        for (const auto& profile : destinationCatalog()) {
            if (profile.available && destination == static_cast<int>(profile.id)) {
                state.selectDestination(profile.id);
                destinationFound = true;
                break;
            }
        }
        if (!destinationFound) {
            return std::nullopt;
        }

        const double loaded = fuel.get<double>();
        const double stepCount = steps.get<double>();
        if (!std::isfinite(loaded) || loaded < 0 || loaded > state.spacecraft().stats().fuelCapacityKg ||
            stepCount < 0 || stepCount > static_cast<double>(state.flightPlan().size())) {
            return std::nullopt;
        }
        const auto completed = static_cast<std::size_t>(stepCount);
        if (phase == static_cast<int>(MissionPhase::Planning)) {
            if (completed != 0 || loaded != 0 || *screen == ScreenType::MissionControl) {
                return std::nullopt;
            }
        } else if (version >= 2) {
            FlightStatus flight;
            const auto& remaining = document.at("remainingFuelKg");
            const bool legacy = version == 2;
            if (phase < static_cast<int>(MissionPhase::InProgress) ||
                phase > static_cast<int>(MissionPhase::Failed) || !remaining.is_number() ||
                !readFlightStatus(document.at("flight"), flight, version.get<int>())) {
                return std::nullopt;
            }
            if (legacy && completed > 0 && completed < state.flightPlan().size() &&
                flight.progress > 0 && flight.progress <= 1) {
                // A leg already underway in the previous simulator had automatic propulsion.
                flight.burnFuelKg = state.flightPlan()[completed].fuelKg;
            }
            if (!state.restoreMission(static_cast<MissionPhase>(phase.get<int>()), completed,
                                      loaded, remaining.get<double>(), flight)) {
                return std::nullopt;
            }
        } else {
            if (!state.beginMission()) {
                return std::nullopt;
            }
            state.adjustFuelLoad(loaded - state.loadedFuelKg());
            // Replay the recorded burns through the normal flight rules.
            for (std::size_t i = 0; i < completed; ++i) {
                state.advanceMission();
            }
            if (phase == static_cast<int>(MissionPhase::Failed)) {
                state.advanceMission();
            }
            if (state.completedSteps() != completed || phase != static_cast<int>(state.missionPhase())) {
                return std::nullopt;
            }
        }
        return SavedGame{std::move(state), *screen};
    } catch (const json::exception&) {
        return std::nullopt;
    }
}

bool SaveManager::deleteSave() const
{
    std::error_code error;
    return hasSave() && std::filesystem::remove(path, error);
}
