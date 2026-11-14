#pragma once

#include <SDL3/SDL.h>
#include "AssetManager.h"

namespace screenDrawing {
inline constexpr SDL_Color background{10, 18, 31, 255};
inline constexpr SDL_Color panel{17, 29, 46, 255};
inline constexpr SDL_Color control{24, 45, 65, 255};
inline constexpr SDL_Color border{47, 77, 100, 255};
inline constexpr SDL_Color text{235, 244, 250, 255};
inline constexpr SDL_Color muted{158, 184, 203, 255};
inline constexpr SDL_Color cyan{93, 224, 248, 255};
inline constexpr SDL_Color green{130, 235, 176, 255};
inline constexpr SDL_Color amber{255, 197, 111, 255};

inline void setColor(SDL_Renderer* renderer, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

inline void drawBox(SDL_Renderer* renderer, const SDL_FRect& bounds,
             SDL_Color fill, SDL_Color outline)
{
    setColor(renderer, fill);
    SDL_RenderFillRect(renderer, &bounds);
    setColor(renderer, outline);
    SDL_RenderRect(renderer, &bounds);
}

inline void drawText(SDL_Renderer* renderer, float x, float y, const char* label,
              SDL_Color color = text, float size = 2)
{
    if (!*label) {
        return;
    }
    if (auto* assets = AssetManager::forRenderer(renderer);
        assets && assets->drawText(x, y, label, color, size)) {
        return;
    }
    float scaleX, scaleY;
    SDL_GetRenderScale(renderer, &scaleX, &scaleY);
    SDL_SetRenderScale(renderer, scaleX * size, scaleY * size);
    setColor(renderer, color);
    SDL_RenderDebugText(renderer, x / size, y / size, label);
    SDL_SetRenderScale(renderer, scaleX, scaleY);
}

inline SDL_FPoint pointerPosition(SDL_Renderer* renderer)
{
    SDL_FPoint mouse;
    SDL_GetMouseState(&mouse.x, &mouse.y);
    SDL_RenderCoordinatesFromWindow(renderer, mouse.x, mouse.y, &mouse.x, &mouse.y);
    return mouse;
}

inline void drawButton(SDL_Renderer* renderer, const SDL_FRect& bounds, const SDL_FPoint& mouse,
                const char* label, bool enabled, bool primary = false)
{
    const bool hovered = enabled && SDL_PointInRectFloat(&mouse, &bounds);
    const bool active = enabled && (primary || hovered);
    const bool pressed = hovered && (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK);
    const SDL_Color fill = pressed ? SDL_Color{27, 83, 106, 255} :
                            active ? SDL_Color{24, 66, 86, 255} : enabled ? control : panel;
    drawBox(renderer, bounds, fill, active ? cyan : border);
    drawText(renderer, bounds.x + 16, bounds.y + (bounds.h - 26) / 2,
             label, enabled ? text : muted, 1.75f);
}

}
