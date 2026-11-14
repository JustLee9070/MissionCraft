#pragma once

#include <filesystem>

struct DisplaySettings {
    int width = 1200;
    int height = 800;
    bool fullscreen = false;
    bool vsync = true;
    int masterVolume = 80;
};

class OptionsManager {
public:
    explicit OptionsManager(std::filesystem::path path = "config/options.json");

    // Missing or malformed settings fall back to defaults.
    void load();
    bool save() const;
    const DisplaySettings& getSettings() const;
    void setSettings(DisplaySettings settings);
    void resetDefaults();

private:
    static DisplaySettings validateSettings(DisplaySettings settings);

    std::filesystem::path path;
    DisplaySettings settings;
};
