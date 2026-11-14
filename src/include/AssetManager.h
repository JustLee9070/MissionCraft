#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <map>
#include <memory>
#include <string>
#include <tuple>

class AssetManager {
public:
    explicit AssetManager(SDL_Renderer* renderer);
    ~AssetManager();

    static AssetManager* forRenderer(SDL_Renderer* renderer);
    SDL_Texture* texture(const char* path);
    bool drawText(float x, float y, const char* label, SDL_Color color, float size);

private:
    using Texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;
    using Font = std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)>;

    SDL_Renderer* renderer;
    std::map<std::string, Texture> textures;
    std::map<float, Font> fonts;
    std::map<std::tuple<float, Uint32, std::string>, Texture> labels;
};
