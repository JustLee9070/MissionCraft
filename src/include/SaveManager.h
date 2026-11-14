#pragma once

#include "GameState.h"
#include "Screen.h"

#include <filesystem>
#include <optional>

struct SavedGame {
    GameState state;
    ScreenType screen;
};

class SaveManager {
public:
    explicit SaveManager(std::filesystem::path path = "saves/save1.json");

    bool hasSave() const;
    bool saveGame(const GameState& state, ScreenType screen) const;
    std::optional<SavedGame> loadGame() const;
    bool deleteSave() const;

private:
    std::filesystem::path path;
};
