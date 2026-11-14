#include "AssetManager.h"

namespace {
constexpr char assetProperty[] = "MissionCraft.AssetManager";
constexpr std::size_t maxCachedLabels = 256;
}

AssetManager::AssetManager(SDL_Renderer* renderer)
    : renderer(renderer)
{
    SDL_SetPointerProperty(SDL_GetRendererProperties(renderer), assetProperty, this);
}

AssetManager::~AssetManager()
{
    SDL_ClearProperty(SDL_GetRendererProperties(renderer), assetProperty);
}

AssetManager* AssetManager::forRenderer(SDL_Renderer* renderer)
{
    return static_cast<AssetManager*>(
        SDL_GetPointerProperty(SDL_GetRendererProperties(renderer), assetProperty, nullptr));
}

SDL_Texture* AssetManager::texture(const char* path)
{
    auto it = textures.find(path);
    if (it == textures.end()) {
        auto* surface = SDL_LoadPNG(path);
        auto* image = surface ? SDL_CreateTextureFromSurface(renderer, surface) : nullptr;
        SDL_DestroySurface(surface);
        if (image) {
            SDL_SetTextureScaleMode(image, SDL_SCALEMODE_NEAREST);
        }
        it = textures.emplace(path, Texture(image, SDL_DestroyTexture)).first;
    }
    return it->second.get();
}

bool AssetManager::drawText(float x, float y, const char* label, SDL_Color color, float size)
{
    const Uint32 rgba = (static_cast<Uint32>(color.r) << 24) |
                        (static_cast<Uint32>(color.g) << 16) |
                        (static_cast<Uint32>(color.b) << 8) | color.a;
    const auto key = std::make_tuple(size, rgba, std::string(label));
    auto it = labels.find(key);
    if (it == labels.end()) {
        auto font = fonts.find(size);
        if (font == fonts.end()) {
            const char* path = size >= 3.5f ? "assets/fonts/Orbitron-Bold.ttf"
                                            : "assets/fonts/RobotoMono-Regular.ttf";
            font = fonts.emplace(size, Font(TTF_OpenFont(path, 12 * size), TTF_CloseFont)).first;
        }
        if (!font->second) {
            return false;
        }
        auto* surface = TTF_RenderText_Blended(font->second.get(), label, 0, color);
        auto* rendered = surface ? SDL_CreateTextureFromSurface(renderer, surface) : nullptr;
        SDL_DestroySurface(surface);
        if (labels.size() >= maxCachedLabels) {
            labels.clear();
        }
        it = labels.emplace(key, Texture(rendered, SDL_DestroyTexture)).first;
    }
    auto* rendered = it->second.get();
    SDL_FRect bounds{x, y, 0, 0};
    return rendered && SDL_GetTextureSize(rendered, &bounds.w, &bounds.h) &&
           SDL_RenderTexture(renderer, rendered, nullptr, &bounds);
}
