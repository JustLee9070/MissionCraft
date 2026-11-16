
#include "screens/PlannerScreen.h"
#include "SpaceArtwork.h"
#include "AssetManager.h"

#include <algorithm>
#include <cstdio>

namespace {
using namespace screenDrawing;

constexpr SDL_FRect destinationsPanel{32, 140, 356, 534};
constexpr SDL_FRect requirementsPanel{408, 140, 760, 260};
constexpr SDL_FRect vehiclesPanel{408, 416, 760, 258};
constexpr SDL_FRect backButton{32, 708, 296, 52};
constexpr SDL_FRect startButton{850, 708, 318, 52};

SDL_FRect destinationRow(std::size_t index)
{
    return {48, 224 + 116 * static_cast<float>(index), 324, 92};
}

SDL_FRect vehicleCard(std::size_t index)
{
    return {420 + 186 * static_cast<float>(index), 454, 178, 210};
}
}

PlannerScreen::PlannerScreen(GameState& gameState)
    : state(gameState)
{
}

void PlannerScreen::startMission()
{
    if (state.beginMission()) {
        requestScreenChange(ScreenType::MissionControl);
    }
}

void PlannerScreen::handleEvents(const SDL_Event& event)
{
    const auto& destinations = destinationCatalog();
    const auto& vehicles = launchVehicleCatalog();

    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        const auto key = event.key.key;

        if (key == SDLK_ESCAPE) {
            requestScreenChange(ScreenType::Designer);
        } else if (key == SDLK_RETURN) {
            startMission();
        } else if (key >= SDLK_1 && key < SDLK_1 + destinations.size()) {
            state.selectDestination(destinations[key - SDLK_1].id);
        } else if (key == SDLK_Q) {
            state.selectLaunchVehicle(LaunchVehicleId::SSLV);
        } else if (key == SDLK_W) {
            state.selectLaunchVehicle(LaunchVehicleId::PSLV);
        } else if (key == SDLK_E) {
            state.selectLaunchVehicle(LaunchVehicleId::GSLV);
        } else if (key == SDLK_R) {
            state.selectLaunchVehicle(LaunchVehicleId::Falcon9);
        }

        return;
    }

    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.button.button != SDL_BUTTON_LEFT) {
        return;
    }

    const SDL_FPoint click{event.button.x, event.button.y};

    if (SDL_PointInRectFloat(&click, &backButton)) {
        requestScreenChange(ScreenType::Designer);
        return;
    }

    if (SDL_PointInRectFloat(&click, &startButton)) {
        startMission();
        return;
    }

    for (std::size_t i = 0; i < destinations.size(); ++i) {
        const auto row = destinationRow(i);

        if (SDL_PointInRectFloat(&click, &row)) {
            state.selectDestination(destinations[i].id);
            return;
        }
    }

    for (std::size_t i = 0; i < vehicles.size(); ++i) {
        const auto card = vehicleCard(i);

        if (SDL_PointInRectFloat(&click, &card)) {
            const bool compatible = isVehicleCompatible(
                vehicles[i],
                state.destination().id,
                state.launchMassKg()
            );

            if (compatible) {
                state.selectLaunchVehicle(vehicles[i].id);
            }

            return;
        }
    }
}

void PlannerScreen::update(float deltaTime)
{
    (void)deltaTime;
}

void PlannerScreen::render(SDL_Renderer* renderer)
{
    const auto& destinations = destinationCatalog();
    const auto& vehicles = launchVehicleCatalog();

    const auto& destination = state.destination();
    const auto& craft = state.spacecraft();

    const auto checks = missionChecks(destination, craft);
    const auto stats = craft.stats();

    const double launchMass = state.launchMassKg();

    const bool canStart = state.canBeginMission();
    const auto mouse = pointerPosition(renderer);

    spaceArtwork::drawBackdrop(renderer);

    drawText(renderer, 32, 32, "MISSION PLANNER", text, 4);
    drawText(renderer, 32, 88,
             "Choose a destination, review requirements and select a launch vehicle.",
             muted);

    // Destination panel.
    drawBox(renderer, destinationsPanel, panel, border);
    drawText(renderer, 48, 164, "DESTINATIONS", cyan, 2.25f);
    drawText(renderer, 48, 194, "Click or press 1-3 to choose", muted, 1.5f);

    for (std::size_t i = 0; i < destinations.size(); ++i) {
        const auto row = destinationRow(i);
        const auto& entry = destinations[i];

        const bool selected = destination.id == entry.id;
        const bool hovered = entry.available &&
                             SDL_PointInRectFloat(&mouse, &row);

        drawBox(renderer, row,
                entry.available ? control : background,
                selected || hovered ? cyan : border);

        spaceArtwork::drawSprite(
            renderer,
            spaceArtwork::planet(entry.id),
            {row.x + 14, row.y + 13, 66, 66},
            entry.available ? 255 : 185
        );

        drawText(renderer, row.x + 94, row.y + 14,
                 entry.name, entry.available ? text : muted, 2.0f);

        drawText(renderer, row.x + 94, row.y + 54,
                 entry.difficulty, muted, 1.5f);

        if (selected || !entry.available) {
            drawText(renderer, row.x + 220, row.y + 54,
                     selected ? "Selected" : "Locked",
                     selected ? cyan : amber, 1.5f);
        }
    }

    // Mission requirements panel.
    drawBox(renderer, requirementsPanel, panel, border);

    drawText(renderer, 432, 158, destination.name, text, 2.5f);
    drawText(renderer, 432, 190, destination.objective, muted, 1.4f);

    spaceArtwork::drawSprite(
        renderer,
        spaceArtwork::planet(destination.id),
        {1060, 150, 76, 76}
    );

    char detail[100];

    std::snprintf(
        detail, sizeof(detail),
        "Launch mass: %.0f kg    Power balance: %+.0f W",
        launchMass,
        stats.powerBalanceW()
    );

    drawText(renderer, 432, 218, detail, cyan, 1.35f);

    drawText(renderer, 432, 248, "FLIGHT REQUIREMENTS", cyan, 1.5f);
    drawText(renderer, 1010, 248, "STATUS", cyan, 1.5f);

    for (std::size_t i = 0; i < checks.size(); ++i) {
        const auto& check = checks[i];

        const SDL_FRect row{
            432,
            272 + 20 * static_cast<float>(i),
            712,
            19
        };

        drawText(renderer, row.x + 8, row.y + 2,
                 check.label, text, 1.25f);

        drawText(
            renderer,
            1010,
            row.y + 2,
            !check.required ? "Optional" : check.met ? "Met" : "Unmet",
            !check.required ? muted : check.met ? green : amber,
            1.25f
        );
    }

    // Launch vehicle selection panel.
    drawBox(renderer, vehiclesPanel, panel, border);

    drawText(renderer, 432, 422, "SELECT LAUNCH VEHICLE", cyan, 1.8f);
    drawText(renderer, 432, 442,
             "Q / W / E / R or click a compatible vehicle",
             muted, 1.15f);

    auto* assets = AssetManager::forRenderer(renderer);

    for (std::size_t i = 0; i < vehicles.size(); ++i) {
        const auto& vehicle = vehicles[i];
        const auto card = vehicleCard(i);

        const bool selected = state.launchVehicleId() == vehicle.id;
        const bool hovered = SDL_PointInRectFloat(&mouse, &card);

        const double capacity = payloadCapacity(
            vehicle, destination.id
        );

        const bool compatible = isVehicleCompatible(
            vehicle, destination.id, launchMass
        );

        SDL_Color cardBorder = selected ? cyan :
                               hovered && compatible ? cyan :
                               compatible ? border : amber;

        drawBox(
            renderer,
            card,
            selected ? control : background,
            cardBorder
        );

        // Rocket image.
        if (assets) {
            SDL_Texture* texture = assets->texture(vehicle.imagePath);

            if (texture) {
                float imageWidth = 0;
                float imageHeight = 0;

                if (SDL_GetTextureSize(texture, &imageWidth, &imageHeight) &&
                    imageWidth > 0 && imageHeight > 0) {

                    constexpr float maxWidth = 82;
                    constexpr float maxHeight = 70;

                    const float scale = std::min(
                        maxWidth / imageWidth,
                        maxHeight / imageHeight
                    );

                    const float width = imageWidth * scale;
                    const float height = imageHeight * scale;

                    SDL_FRect imageRect{
                        card.x + (card.w - width) / 2,
                        card.y + 4 + (maxHeight - height) / 2,
                        width,
                        height
                    };

                    SDL_RenderTexture(renderer, texture, nullptr, &imageRect);
                }
            }
        }

        // Vehicle name.
        drawText(
            renderer,
            card.x + 10,
            card.y + 78,
            vehicle.name,
            selected ? cyan : text,
            1.55f
        );

        // Payload capacity.
        std::snprintf(
            detail, sizeof(detail),
            "Capacity: %.0f kg",
            capacity
        );

        drawText(renderer, card.x + 10, card.y + 106,
                 detail, muted, 1.05f);

        // Launch cost.
        std::snprintf(
            detail, sizeof(detail),
            "Cost: $%.0fM",
            vehicle.cost / 1000000.0
        );

        drawText(renderer, card.x + 10, card.y + 126,
                 detail, muted, 1.05f);

        // Reliability.
        std::snprintf(
            detail, sizeof(detail),
            "Reliability: %.0f%%",
            vehicle.reliability * 100
        );

        drawText(renderer, card.x + 10, card.y + 146,
                 detail, muted, 1.05f);

        // Reusability.
        drawText(
            renderer,
            card.x + 10,
            card.y + 166,
            vehicle.reusable ? "Reusable" : "Expendable",
            muted,
            1.05f
        );

        // Compatibility status.
        const char* status = nullptr;
        SDL_Color statusColor = muted;

        if (capacity <= 0) {
            status = "Unavailable";
            statusColor = amber;
        } else if (launchMass > capacity) {
            status = "Over capacity";
            statusColor = amber;
        } else {
            status = selected ? "Selected" : "Compatible";
            statusColor = green;
        }

        drawText(
            renderer,
            card.x + 10,
            card.y + 188,
            status,
            statusColor,
            1.05f
        );
    }

    // Bottom buttons.
    drawButton(renderer, backButton, mouse, "< Designer (Esc)", true);

    const auto phase = state.missionPhase();

    const char* action =
        phase == MissionPhase::InProgress ? "Resume mission (Enter)" :
        phase == MissionPhase::Planning ? "Start mission (Enter)" :
        "Fly again (Enter)";

    drawButton(renderer, startButton, mouse, action, canStart, true);
}
