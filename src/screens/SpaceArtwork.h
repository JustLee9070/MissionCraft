#pragma once

#include "Destination.h"
#include "ScreenDrawing.h"

namespace spaceArtwork {
inline constexpr char starfield[] = "assets/images/backgrounds/starfield.png";
inline constexpr char nebula[] = "assets/images/backgrounds/nebula.png";
inline constexpr char launchSky[] = "assets/images/backgrounds/launch-sky.png";
inline constexpr char lunarTransfer[] = "assets/images/backgrounds/lunar-transfer.png";
inline constexpr char earth[] = "assets/images/planets/earth.png";
inline constexpr char moon[] = "assets/images/planets/moon.png";
inline constexpr char mars[] = "assets/images/planets/mars.png";
inline constexpr char sun[] = "assets/images/planets/sun.png";
inline constexpr char ship[] = "assets/images/playerShip.png";

inline const char* planet(Destination destination)
{
    switch (destination) {
    case Destination::EarthOrbit: return earth;
    case Destination::Moon: return moon;
    case Destination::Mars: return mars;
    }
    return earth;
}

inline SDL_Texture* texture(SDL_Renderer* renderer, const char* path)
{
    auto* assets = AssetManager::forRenderer(renderer);
    return assets ? assets->texture(path) : nullptr;
}

inline void drawSprite(SDL_Renderer* renderer, const char* path, const SDL_FRect& bounds,
                       Uint8 brightness = 255)
{
    if (auto* image = texture(renderer, path)) {
        SDL_SetTextureColorMod(image, brightness, brightness, brightness);
        SDL_RenderTexture(renderer, image, nullptr, &bounds);
        SDL_SetTextureColorMod(image, 255, 255, 255);
    }
}

inline void drawSky(SDL_Renderer* renderer, const char* path, const SDL_FRect& bounds,
                    Uint8 brightness)
{
    auto* image = texture(renderer, path);
    float width, height;
    if (!image || !SDL_GetTextureSize(image, &width, &height)) {
        return;
    }
    SDL_SetTextureScaleMode(image, SDL_SCALEMODE_LINEAR);
    const float aspect = bounds.w / bounds.h;
    SDL_FRect crop{0, 0, width, height};
    if (width / height > aspect) {
        crop.w = height * aspect;
        crop.x = (width - crop.w) * 0.5f;
    } else {
        crop.h = width / aspect;
        crop.y = (height - crop.h) * 0.5f;
    }
    SDL_SetTextureColorMod(image, brightness, brightness, brightness);
    SDL_RenderTexture(renderer, image, &crop, &bounds);
    SDL_SetTextureColorMod(image, 255, 255, 255);
}

inline void drawBackdrop(SDL_Renderer* renderer, const char* path = starfield,
                         Uint8 brightness = 190)
{
    screenDrawing::setColor(renderer, screenDrawing::background);
    SDL_RenderClear(renderer);
    drawSky(renderer, path, {0, 0, 1200, 800}, brightness);
}
}
