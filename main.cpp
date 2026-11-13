
#include "./src/include/Game.h"
#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Game game;

    if (!game.initialize()) {
        std::cerr << "Failed to initialize MissionCraft: "
                  << SDL_GetError() << '\n';
        return 1;
    }

    std::cout << "MissionCraft initialized successfully.\n";

    game.run();

    std::cout << "MissionCraft shutting down.\n";

    return 0;
}
