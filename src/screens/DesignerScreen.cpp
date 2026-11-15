
#include "screens/DesignerScreen.h"
#include "SpaceArtwork.h"

#include <algorithm>
#include <array>
#include <cstdio>

namespace {
using namespace screenDrawing;

constexpr SDL_FRect libraryPanel{32, 140, 394, 544};
constexpr SDL_FRect assemblyPanel{442, 140, 342, 544};
constexpr SDL_FRect telemetryPanel{800, 140, 368, 544};
constexpr SDL_FRect backButton{32, 708, 204, 52};
constexpr SDL_FRect clearButton{256, 708, 236, 52};
constexpr SDL_FRect removeButton{516, 708, 236, 52};
constexpr SDL_FRect plannerButton{872, 708, 296, 52};
constexpr SDL_FRect standardConfigButton{820, 190, 156, 26};
constexpr SDL_FRect lightweightConfigButton{988, 190, 160, 26};
constexpr float installDuration = 0.55f;
constexpr float assemblyCenter = 613;

struct AssemblyPart {
    ComponentId id;
    float imageHeight;
    float topConnector;
    float bottomConnector;
    float scale;
};

// Connector positions are measured down the center of the 384-pixel-wide artwork.
constexpr std::array<AssemblyPart, 7> assemblyOrder{{
    {ComponentId::HighGainAntenna, 192, 4, 188, 0.30f},
    {ComponentId::SciencePayload, 192, 10, 182, 0.29f},
    {ComponentId::CommandModule, 256, 22, 244, 0.31f},
    {ComponentId::SolarArray, 128, 25, 105, 0.78f},
    {ComponentId::BatteryPack, 192, 20, 168, 0.29f},
    {ComponentId::FuelTank, 128, 16, 120, 0.36f},
    {ComponentId::ChemicalEngine, 359, 14, 345, 0.30f}
}};

struct AssemblySlot {
    SDL_FRect image;
    SDL_FRect hitArea;
};

std::size_t libraryIndex(ComponentId id)
{
    const auto& catalog = componentCatalog();
    return std::distance(catalog.begin(), std::find_if(catalog.begin(), catalog.end(),
        [id](const Component& part) { return part.id == id; }));
}

float ease(float progress)
{
    return progress * progress * (3 - 2 * progress);
}

auto assemblySlots(const std::array<float, 7>& slotProgress)
{
    std::array<AssemblySlot, assemblyOrder.size()> slots{};
    float joint = 237;

    for (std::size_t i = 0; i < slots.size(); ++i) {
        const auto& part = assemblyOrder[i];
        const float width = 384 * part.scale;
        const float height = part.imageHeight * part.scale;
        const float connectionSpan = (part.bottomConnector - part.topConnector) * part.scale;

        slots[i] = {
            {assemblyCenter - width / 2, joint - part.topConnector * part.scale,
             width, height},
            {assemblyCenter - width / 2, joint, width, connectionSpan - 2}
        };

        joint += (connectionSpan - 2) * ease(slotProgress[libraryIndex(part.id)]);
    }

    return slots;
}

SDL_FRect libraryRow(std::size_t index)
{
    return {48, 222 + 63 * static_cast<float>(index), 362, 57};
}

SDL_FRect thumbnailFrame(const SDL_FRect& row)
{
    return {row.x + 8, row.y + 4, 116, row.h - 8};
}

const char* componentImage(ComponentId id)
{
    switch (id) {
    case ComponentId::CommandModule:
        return "assets/images/components/command-module.png";
    case ComponentId::ChemicalEngine:
        return "assets/images/components/chemical-engine.png";
    case ComponentId::FuelTank:
        return "assets/images/components/fuel-tank.png";
    case ComponentId::SolarArray:
        return "assets/images/components/solar-array.png";
    case ComponentId::BatteryPack:
        return "assets/images/components/battery-pack.png";
    case ComponentId::HighGainAntenna:
        return "assets/images/components/high-gain-antenna.png";
    case ComponentId::SciencePayload:
        return "assets/images/components/science-payload.png";
    }

    return nullptr;
}

void drawComponentImage(SDL_Renderer* renderer, ComponentId id, const SDL_FRect& frame,
                        const SDL_FRect* destination = nullptr, float progress = 0)
{
    const char* path = componentImage(id);
    auto* image = path ? spaceArtwork::texture(renderer, path) : nullptr;

    float width, height;

    if (!image || !SDL_GetTextureSize(image, &width, &height)) {
        return;
    }

    const float scale = std::min(frame.w / width, frame.h / height);

    SDL_FRect fitted{
        frame.x + (frame.w - width * scale) / 2,
        frame.y + (frame.h - height * scale) / 2,
        width * scale,
        height * scale
    };

    if (destination) {
        fitted.x += (destination->x - fitted.x) * progress;
        fitted.y += (destination->y - fitted.y) * progress
                    - 24 * progress * (1 - progress);
        fitted.w += (destination->w - fitted.w) * progress;
        fitted.h += (destination->h - fitted.h) * progress;
    }

    SDL_SetTextureScaleMode(image, SDL_SCALEMODE_LINEAR);
    SDL_RenderTexture(renderer, image, nullptr, &fitted);
}

const Component* findInstalled(const Spacecraft& craft, ComponentId id)
{
    const auto& parts = craft.components();

    const auto match = std::find_if(parts.begin(), parts.end(),
        [id](const Component& part) { return part.id == id; });

    return match == parts.end() ? nullptr : &match->get();
}

void drawValue(SDL_Renderer* renderer, float y, const char* label, double value,
               const char* format, SDL_Color color = text)
{
    char buffer[48];
    std::snprintf(buffer, sizeof(buffer), format, value);

    drawText(renderer, 820, y, label, muted, 1.5f);
    drawText(renderer, 1016, y, buffer, color, 1.75f);
}
}

DesignerScreen::DesignerScreen(GameState& gameState)
    : state(gameState)
{
    for (const Component& part : state.spacecraft().components()) {
        slotProgress[libraryIndex(part.id)] = 1;
    }
}

void DesignerScreen::addPart(ComponentId id)
{
    if (state.spacecraft().addComponent(id)) {
        installTime[libraryIndex(id)] = installDuration;
        state.returnToPlanning();
    }

    selectedPart = id;
}

void DesignerScreen::removeSelectedPart()
{
    if (selectedPart && state.spacecraft().removeComponent(*selectedPart)) {
        installTime[libraryIndex(*selectedPart)] = 0;
        selectedPart.reset();
        state.returnToPlanning();
    }
}

void DesignerScreen::openPlanner()
{
    if (state.spacecraft().isReady()) {
        requestScreenChange(ScreenType::Planner);
    }
}

void DesignerScreen::handleEvents(const SDL_Event& event)
{
    const auto& catalog = componentCatalog();

    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        const SDL_Keycode key = event.key.key;

        if (key == SDLK_ESCAPE) {
            requestScreenChange(ScreenType::MainMenu);
        } else if (key == SDLK_RETURN) {
            openPlanner();
        } else if (key == SDLK_DELETE) {
            removeSelectedPart();
        } else if (key >= SDLK_1 && key < SDLK_1 + catalog.size()) {
            addPart(catalog[key - SDLK_1].id);
        }

        return;
    }

    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.button.button != SDL_BUTTON_LEFT) {
        return;
    }

    const SDL_FPoint click{event.button.x, event.button.y};

    if (SDL_PointInRectFloat(&click, &backButton)) {
        requestScreenChange(ScreenType::MainMenu);
    } else if (SDL_PointInRectFloat(&click, &clearButton)) {
        if (!state.spacecraft().components().empty()) {
            state.spacecraft() = Spacecraft{};
            installTime.fill(0);
            slotProgress.fill(0);
            selectedPart.reset();
            state.returnToPlanning();
        }
    } else if (SDL_PointInRectFloat(&click, &removeButton)) {
        removeSelectedPart();
    } else if (SDL_PointInRectFloat(&click, &plannerButton)) {
        openPlanner();
    } else if (SDL_PointInRectFloat(&click, &standardConfigButton)) {
        if (state.spacecraft().configuration() != SpacecraftConfiguration::Standard) {
            state.spacecraft().setConfiguration(SpacecraftConfiguration::Standard);
            state.returnToPlanning();
        }
    } else if (SDL_PointInRectFloat(&click, &lightweightConfigButton)) {
        if (state.spacecraft().configuration() != SpacecraftConfiguration::Lightweight) {
            state.spacecraft().setConfiguration(SpacecraftConfiguration::Lightweight);
            state.returnToPlanning();
        }
    } else {
        for (std::size_t i = 0; i < catalog.size(); ++i) {
            const auto row = libraryRow(i);

            if (SDL_PointInRectFloat(&click, &row)) {
                addPart(catalog[i].id);
                return;
            }
        }

        const auto slots = assemblySlots(slotProgress);

        for (std::size_t i = 0; i < assemblyOrder.size(); ++i) {
            const auto id = assemblyOrder[i].id;

            if (SDL_PointInRectFloat(&click, &slots[i].hitArea) &&
                installTime[libraryIndex(id)] == 0 &&
                state.spacecraft().hasComponent(id)) {
                selectedPart = id;
                return;
            }
        }
    }
}

void DesignerScreen::update(float deltaTime)
{
    const auto& catalog = componentCatalog();
    const float reflow = deltaTime / installDuration;

    for (std::size_t i = 0; i < catalog.size(); ++i) {
        installTime[i] = std::max(0.0f, installTime[i] - deltaTime);

        if (state.spacecraft().hasComponent(catalog[i].id)) {
            slotProgress[i] = std::min(1.0f, slotProgress[i] + reflow);

            if (installTime[i] == 0) {
                slotProgress[i] = 1;
            }
        } else {
            slotProgress[i] = std::max(0.0f, slotProgress[i] - reflow);
        }
    }
}

void DesignerScreen::render(SDL_Renderer* renderer)
{
    const auto& craft = state.spacecraft();
    const auto& catalog = componentCatalog();
    const auto stats = craft.stats();
    const auto issues = craft.readinessIssues();
    const auto mouse = pointerPosition(renderer);
    const auto slots = assemblySlots(slotProgress);

    spaceArtwork::drawBackdrop(renderer);

    drawText(renderer, 32, 32, "SPACECRAFT DESIGNER", text, 4);
    drawText(renderer, 32, 88,
             "Assemble an orbiter and pass the readiness checks.", muted);

    drawText(renderer, 872, 34, "PLANNED DESTINATION", muted);
    drawText(renderer, 872, 64, state.destinationName(), cyan, 3);

    drawBox(renderer, libraryPanel, panel, border);
    drawBox(renderer, assemblyPanel, panel, border);
    drawBox(renderer, telemetryPanel, panel, border);

    drawText(renderer, 48, 164, "COMPONENT LIBRARY", cyan, 2.25f);
    drawText(renderer, 48, 194, "Click or press 1-7 to add", muted, 1.5f);

    drawText(renderer, 458, 164, "SPACECRAFT ASSEMBLY", cyan, 2.0f);
    drawText(renderer, 458, 194, "Select a part, then hit Remove", muted, 1.375f);

    drawText(renderer, 820, 164, "LIVE TELEMETRY", cyan, 2.25f);

    // Spacecraft configuration selector
    const bool standardSelected =
        craft.configuration() == SpacecraftConfiguration::Standard;

    const bool lightweightSelected =
        craft.configuration() == SpacecraftConfiguration::Lightweight;

    drawBox(renderer, standardConfigButton, control,
            standardSelected ? cyan : border);

    drawBox(renderer, lightweightConfigButton, control,
            lightweightSelected ? cyan : border);

    drawText(renderer, 837, 196, "STANDARD",
             standardSelected ? cyan : text, 1.25f);

    drawText(renderer, 1002, 196, "LIGHTWEIGHT",
             lightweightSelected ? cyan : text, 1.25f);

    // Component library
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        const auto row = libraryRow(i);
        const auto& component = catalog[i];

        const bool installed = findInstalled(craft, component.id) != nullptr;
        const bool hovered = SDL_PointInRectFloat(&mouse, &row);
        const bool selected = selectedPart == component.id;

        drawBox(renderer, row, control, hovered || selected ? cyan : border);

        const auto imageFrame = thumbnailFrame(row);
        drawBox(renderer, imageFrame, background, border);

        if (installTime[i] == 0) {
            drawComponentImage(renderer, component.id, imageFrame);
        }

        const char number[] = {static_cast<char>('1' + i), '\0'};

        drawText(renderer, row.x + 136, row.y + 7,
                 component.name, text, 1.75f);

        drawText(renderer, row.x + 136, row.y + 34,
                 installed ? "Installed" : "+ Add part",
                 installed ? green : muted, 1.25f);

        drawText(renderer, row.x + row.w - 24, row.y + 34,
                 number, cyan, 1.25f);
    }

    // Spacecraft assembly
    setColor(renderer, border);
    SDL_RenderLine(renderer, assemblyCenter, 237, assemblyCenter, 641);

    for (std::size_t i = 0; i < assemblyOrder.size(); ++i) {
        const auto id = assemblyOrder[i].id;
        const auto& slot = slots[i];

        if (!craft.hasComponent(id) || installTime[libraryIndex(id)] > 0) {
            continue;
        }

        if (selectedPart == id || SDL_PointInRectFloat(&mouse, &slot.hitArea)) {
            setColor(renderer, cyan);
            SDL_RenderRect(renderer, &slot.hitArea);
        }

        drawComponentImage(renderer, id, slot.image);
    }

    // Live telemetry
    drawValue(renderer, 222, "Dry mass", stats.dryMassKg, "%.1f kg");
    drawValue(renderer, 256, "Power generated", stats.powerGenerationW, "%.1f W");
    drawValue(renderer, 290, "Power draw", stats.powerConsumptionW, "%.1f W");

    drawValue(renderer, 324, "Power balance", stats.powerBalanceW(), "%+.1f W",
              stats.powerBalanceW() < 0 ? amber : green);

    drawValue(renderer, 358, "Fuel capacity", stats.fuelCapacityKg, "%.1f kg");

    drawValue(renderer, 392, "Parts installed",
              static_cast<double>(craft.components().size()), "%.0f");

    setColor(renderer, border);
    SDL_RenderLine(renderer, 820, 432, 1148, 432);

    // Flight readiness
    drawText(renderer, 820, 452, "FLIGHT READINESS", cyan, 2.25f);

    drawText(renderer, 820, 487,
             issues.empty() ? "Ready to plan" : "Changes required",
             issues.empty() ? green : amber);

    for (std::size_t i = 0; i < issues.size(); ++i) {
        drawText(renderer, 820, 526 + 27 * static_cast<float>(i),
                 issues[i], text, 1.25f);
    }

    // Navigation buttons
    drawButton(renderer, backButton, mouse, "< Back (Esc)", true);
    drawButton(renderer, clearButton, mouse,
               "Clear all parts", !craft.components().empty());

    drawButton(renderer, removeButton, mouse,
               "Remove (Del)", selectedPart.has_value());

    drawButton(renderer, plannerButton, mouse,
               "Mission Planner >", issues.empty(), true);

    // Component installation animation
    for (std::size_t i = 0; i < assemblyOrder.size(); ++i) {
        const auto id = assemblyOrder[i].id;
        const auto index = libraryIndex(id);

        if (installTime[index] > 0) {
            const float time = 1 - installTime[index] / installDuration;

            drawComponentImage(renderer, id,
                thumbnailFrame(libraryRow(index)),
                &slots[i].image, ease(time));
        }
    }
}
