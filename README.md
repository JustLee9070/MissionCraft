# MissionCraft

### Design. Engineer. Explore.

MissionCraft is a cartoon-inspired spacecraft engineering and space exploration simulation game built with **C++ and SDL3**.

The goal is to let players design their own spacecraft, experiment with different spacecraft components, manage engineering constraints, and prepare missions to destinations across the solar system.

From assembling a spacecraft to planning an interplanetary mission, MissionCraft combines aerospace engineering concepts with accessible simulation gameplay.

## About the Project

Spacecraft engineering involves balancing multiple constraints. A spacecraft needs enough thrust, fuel, electrical power, and communication capabilities to complete its mission.

MissionCraft aims to introduce these concepts through an interactive spacecraft-building experience.

Players will be able to:

- Assemble spacecraft using modular components.
- Monitor spacecraft mass and power requirements.
- Balance power generation and consumption.
- Manage fuel capacity and other engineering constraints.
- Select destinations and prepare missions.
- Progress through different mission phases.

The project is currently under development. Features and gameplay mechanics are being implemented incrementally.

## Technology Stack

| Technology | Purpose                                         |
| ---------- | ----------------------------------------------- |
| C++17      | Core game development                           |
| SDL3       | Window management, rendering and input handling |
| SDL3_image | Image and texture loading                       |
| SDL3_ttf   | Font rendering                                  |
| Make       | Build automation                                |

## Project Structure

```text
MissionCraft/
│
├── assets/
│   └── fonts/
│
├── src/
│   ├── include/
│   │   ├── Game.h
│   │   ├── Screen.h
│   │   └── screens/
│   │       ├── TitleScreen.h
│   │       ├── MainMenu.h
│   │       ├── DesignerScreen.h
│   │       ├── PlannerScreen.h
│   │       ├── MissionControl.h
│   │       └── OptionsScreen.h
│   │
│   ├── screens/
│   │   ├── TitleScreen.cpp
│   │   ├── MainMenu.cpp
│   │   ├── DesignerScreen.cpp
│   │   ├── PlannerScreen.cpp
│   │   ├── MissionControl.cpp
│   │   └── OptionsScreen.cpp
│   │
│   └── Game.cpp
│
├── Makefile
├── main.cpp
└── README.md
```

## Getting Started

### Prerequisites

The project currently targets Windows with the MSYS2 UCRT64 environment.

You will need:

- A C++17-compatible compiler (GCC recommended).
- GNU Make.
- SDL3.
- SDL3_image.
- SDL3_ttf.
- nlohmann/json.

### Installation

**1. Install the required SDL libraries**

Open the MSYS2 UCRT64 terminal and run:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc \
          mingw-w64-ucrt-x86_64-make \
          mingw-w64-ucrt-x86_64-sdl3 \
          mingw-w64-ucrt-x86_64-sdl3-image \
          mingw-w64-ucrt-x86_64-sdl3-ttf \
          mingw-w64-ucrt-x86_64-nlohmann-json
```

**2. Clone the repository**

```bash
git clone https://github.com/JustLee9070/MissionCraft.git
cd MissionCraft
```

**3. Build the project**

```bash
make
```

**4. Run the game**

```bash
./MissionCraft.exe
```

If you're running the executable from PowerShell, make sure the MSYS2 UCRT64 DLL directory is in your PATH:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
```

Then run:

```powershell
.\MissionCraft.exe
```

## Development Roadmap

MissionCraft is actively being developed. The following features represent the planned development direction.

- [x] SDL3 project setup
- [x] Basic game loop
- [x] Screen-based architecture
- [x] Title screen
- [x] Main menu navigation
- [ ] Functional spacecraft designer
- [ ] Modular spacecraft component system
- [ ] Real-time spacecraft statistics
- [ ] Spacecraft readiness validation
- [ ] Mission destination selection
- [ ] Mission control interface
- [ ] Options and configuration management
- [ ] Save and load functionality
- [ ] Mission progression and engineering simulation
- [ ] Improved graphics, animations and sound
- [ ] Web build using Emscripten and WebAssembly

## Future Development

Some of the engineering mechanics being considered for future versions include:

- Delta-v calculations.
- Thrust-to-weight ratio.
- Orbital transfers and mission trajectories.
- Fuel consumption and staging.
- Battery charge and power management.
- Communication range and spacecraft limitations.

These mechanics will be introduced progressively as the simulation develops.

## Project Goals

The long-term goal is to build an interactive spacecraft engineering simulator that makes fundamental aerospace engineering concepts approachable through gameplay.

MissionCraft is also an opportunity to explore game architecture, object-oriented programming, graphics programming, physics-based simulation, and cross-platform development using C++.

---

**MissionCraft — Design. Engineer. Explore.**
