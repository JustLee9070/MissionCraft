
#include "OptionsManager.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

namespace {
using nlohmann::json;

int readDimension(const json& display, const char* key)
{
    const auto value = display.find(key);
    if (value == display.end() || !value->is_number_integer()) {
        return 0;
    }
    const double size = value->get<double>();
    return size > 0 && size <= 16384 ? static_cast<int>(size) : 0;
}

bool readToggle(const json& display, const char* key, bool fallback)
{
    const auto value = display.find(key);
    return value != display.end() && value->is_boolean()
        ? value->get<bool>() : fallback;
}

int readVolume(const json& audio, const char* key, int fallback)
{
    const auto value = audio.find(key);
    if (value == audio.end() || !value->is_number_integer()) {
        return fallback;
    }
    return std::clamp(value->get<int>(), 0, 100);
}
}

OptionsManager::OptionsManager(std::filesystem::path path)
    : path(std::move(path))
{
}

void OptionsManager::load()
{
    resetDefaults();

    std::ifstream input(path);
    if (!input) {
        return;
    }

    const auto document = json::parse(input, nullptr, false);
    if (document.is_discarded() || !document.is_object()) {
        return;
    }

    const auto display = document.find("display");
    if (display == document.end() || !display->is_object()) {
        return;
    }

    const auto audio = document.find("audio");

    int masterVolume = settings.masterVolume;
    int musicVolume = settings.musicVolume;

    if (audio != document.end() && audio->is_object()) {
        masterVolume = readVolume(*audio, "masterVolume", masterVolume);
        musicVolume = readVolume(*audio, "musicVolume", musicVolume);
    }

    setSettings({
        readDimension(*display, "width"),
        readDimension(*display, "height"),
        readToggle(*display, "fullscreen", settings.fullscreen),
        readToggle(*display, "vsync", settings.vsync),
        masterVolume,
        musicVolume,
        readToggle(*display, "showFPS", settings.showFPS)
    });
}

bool OptionsManager::save() const
{
    std::error_code error;

    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            return false;
        }
    }

    auto temporaryPath = path;
    temporaryPath += ".tmp";

    const json document = {
        {"display", {
            {"width", settings.width},
            {"height", settings.height},
            {"fullscreen", settings.fullscreen},
            {"vsync", settings.vsync},
            {"showFPS", settings.showFPS}
        }},
        {"audio", {
            {"masterVolume", settings.masterVolume},
            {"musicVolume", settings.musicVolume}
        }}
    };

    std::ofstream output(temporaryPath);
    if (!output) {
        return false;
    }

    output << document.dump(2) << '\n';
    output.close();

    if (output) {
        std::filesystem::rename(temporaryPath, path, error);
        if (!error) {
            return true;
        }
    }

    std::filesystem::remove(temporaryPath, error);
    return false;
}

const DisplaySettings& OptionsManager::getSettings() const
{
    return settings;
}

void OptionsManager::setSettings(DisplaySettings newSettings)
{
    settings = validateSettings(newSettings);
}

void OptionsManager::resetDefaults()
{
    settings = {};
}

DisplaySettings OptionsManager::validateSettings(DisplaySettings candidate)
{
    const DisplaySettings defaults;

    if (candidate.width < 640 || candidate.width > 16384) {
        candidate.width = defaults.width;
    }

    if (candidate.height < 480 || candidate.height > 16384) {
        candidate.height = defaults.height;
    }

    candidate.masterVolume = std::clamp(candidate.masterVolume, 0, 100);
    candidate.musicVolume = std::clamp(candidate.musicVolume, 0, 100);

    return candidate;
}
