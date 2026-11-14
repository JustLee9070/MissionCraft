#include "SaveManager.h"

#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <random>
#include <utility>

namespace {
using nlohmann::json;
constexpr int saveVersion = 1;

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
    const json document{
        {"version", saveVersion},
        {"screen", key},
        {"components", std::move(components)},
        {"destination", static_cast<int>(state.destination().id)},
        {"phase", static_cast<int>(state.missionPhase())},
        {"completedSteps", state.completedSteps()},
        {"loadedFuelKg", state.loadedFuelKg()}
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
        if (!version.is_number_integer() || version != saveVersion) {
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
