# MissionCraft

### Design. Engineer. Explore.

**MissionCraft** is a cartoon-inspired spacecraft engineering and space exploration simulation game developed for the **NASA Space Apps Challenge 2026**.

Built with **C++17 and SDL3**, MissionCraft challenges players to design spacecraft, experiment with modular components, manage engineering constraints, select launch vehicles, and prepare missions to destinations across the solar system.

The project aims to make fundamental aerospace engineering concepts accessible through interactive and engaging simulation gameplay.

## About the Project

Spacecraft engineering involves balancing several interconnected constraints. A spacecraft needs sufficient fuel, electrical power, payload capacity, and reliable systems to complete its mission.

Developed as part of the NASA Space Apps Challenge, MissionCraft explores how game development can make spacecraft engineering and space exploration more approachable.

### Key Features

- **Spacecraft Design:** Assemble spacecraft using modular components such as command modules, engines, fuel tanks, solar arrays, batteries, antennas, and scientific payloads.
- **Spacecraft Configurations:** Choose between Standard and Lightweight configurations, each with different mass characteristics.
- **Launch Vehicle Selection:** Choose from four launch vehicles: SSLV, PSLV, GSLV, and Falcon 9.
- **Payload Compatibility:** Evaluate launch vehicle compatibility based on spacecraft mass, fuel requirements, and destination.
- **Resource Management:** Monitor spacecraft mass, fuel capacity, power generation, and power consumption.
- **Mission Planning:** Select destinations and prepare spacecraft for their missions.
- **Mission Control:** Manage simulated mission operations, monitor telemetry, and respond to changing flight conditions.
- **Save and Load:** Preserve spacecraft configurations, selected launch vehicles, and mission progress.
- **Options Management:** Customize display and audio settings.

## Technology Stack

| Technology | Purpose |
|---|---|
| C++17 | Core game logic and simulation |
| SDL3 | Window management, rendering, input handling, and game loop |
| SDL3_image | Image and texture loading |
| SDL3_ttf | Font rendering |
| SDL3_mixer | Audio and sound effects |
| nlohmann/json | JSON-based configuration and save/load management |
| GNU Make | Build automation |
| GCC (MinGW-w64) | C++ compilation |
| MSYS2 UCRT64 | Windows development environment |
| Git & GitHub | Version control and collaboration |

## Project Architecture

MissionCraft uses a modular, screen-based architecture with separate systems for game state, spacecraft management, mission planning, asset loading, audio, and persistent storage.

```text
MissionCraft/
│
├── assets/
│   ├── fonts/
│   ├── images/
│   ├── launch_vehicles/
│   └── ...
│
├── config/
│
├── src/
│   ├── include/
│   │   ├── Game.h
│   │   ├── Screen.h
│   │   ├── GameState.h
│   │   ├── Spacecraft.h
│   │   ├── LaunchVehicle.h
│   │   └── screens/
│   │
│   ├── screens/
│   │   ├── TitleScreen.cpp
│   │   ├── MainMenu.cpp
│   │   ├── DesignerScreen.cpp
│   │   ├── PlannerScreen.cpp
│   │   ├── MissionControl.cpp
│   │   └── OptionsScreen.cpp
│   │
│   ├── Game.cpp
│   ├── GameState.cpp
│   ├── Component.cpp
│   ├── Spacecraft.cpp
│   ├── LaunchVehicle.cpp
│   ├── Destination.cpp
│   ├── AssetManager.cpp
│   ├── SaveManager.cpp
│   ├── OptionsManager.cpp
│   └── GameAudio.cpp
│
├── resources.rc
├── Makefile
├── main.cpp
└── README.md
```

## Getting Started

### Prerequisites

The development version currently targets Windows using the MSYS2 UCRT64 environment.

You will need:

- Windows 10/11 (64-bit)
- GCC with C++17 support
- GNU Make
- SDL3
- SDL3_image
- SDL3_ttf
- SDL3_mixer
- nlohmann/json

### Installation

**1. Install the required dependencies**

Open the MSYS2 UCRT64 terminal and run:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc \
          mingw-w64-ucrt-x86_64-make \
          mingw-w64-ucrt-x86_64-SDL3 \
          mingw-w64-ucrt-x86_64-SDL3_image \
          mingw-w64-ucrt-x86_64-SDL3_ttf \
          mingw-w64-ucrt-x86_64-SDL3_mixer \
          mingw-w64-ucrt-x86_64-nlohmann-json
```

**2. Clone the repository**

```bash
git clone https://github.com/JustLee9070/MissionCraft.git
cd MissionCraft
```

**3. Build the development version**

```bash
make
```

**4. Run the game**

```bash
make run
```

Alternatively:

```bash
./MissionCraft.exe
```

If running the executable from PowerShell, make sure the MSYS2 UCRT64 DLL directory is in your PATH:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
```

Then run:

```powershell
.\MissionCraft.exe
```

## Creating a Windows Release Build

MissionCraft includes a dedicated Makefile target for preparing a distributable Windows build.

Run:

```bash
make build
```

This command creates a `build/` directory containing:

- Optimized release executable.
- Required game assets.
- Configuration files.
- Save directory.
- Runtime DLL dependencies.
- Basic launch instructions.

The contents of this directory can be compressed into a ZIP archive and distributed through GitHub Releases.

**Players can run the packaged executable without installing the development toolchain.**

## Development Roadmap

MissionCraft is actively under development. Future improvements include:

- [x] SDL3 project setup and basic game loop
- [x] Screen-based game architecture
- [x] Title screen and main menu navigation
- [x] Modular spacecraft component system
- [x] Standard and Lightweight spacecraft configurations
- [x] Real-time spacecraft statistics
- [x] Spacecraft readiness validation
- [x] Launch vehicle selection and compatibility checks
- [x] Mission destination selection
- [x] Mission planning and control interfaces
- [x] Options and configuration management
- [x] Save and load functionality
- [x] Basic mission progression and resource management
- [x] Windows release build packaging
- [ ] More advanced aerospace simulation mechanics
- [ ] Improved graphics, animations, and sound
- [ ] More mission types and destinations
- [ ] Web build using Emscripten and WebAssembly

## Future Development

Some of the engineering and simulation mechanics being considered for future versions include:

- Delta-v calculations.
- Thrust-to-weight ratio.
- Orbital transfers and mission trajectories.
- Fuel consumption and spacecraft staging.
- Battery charge and power management.
- Communication range and signal limitations.
- More detailed spacecraft reliability and failure simulation.

These mechanics will be introduced progressively as the simulation develops.

## Project Goals

MissionCraft was developed for the **NASA Space Apps Challenge 2026**, with the goal of exploring how interactive game development can introduce players to spacecraft engineering and space exploration.

The long-term vision is to build an interactive spacecraft engineering simulator that makes fundamental aerospace engineering concepts approachable through gameplay.

The project also provides an opportunity to explore:

- Object-oriented programming and software architecture.
- C++ game development and graphics programming.
- Resource management and physics-based simulation.
- Aerospace engineering fundamentals.
- Cross-platform development and software distribution.

## Credits & Acknowledgements

MissionCraft was developed collaboratively for the **NASA Space Apps Challenge 2026**.

### Development Team

| Team Member | Role & Contributions |
|---|---|
| **Rakshin Joshi** | **Development Lead** – Led overall development, game architecture, core programming, and integration of game systems. |
| **Aryan Gautam** | **Game Developer** – Contributed to game development and implementation. |
| **Ojas Kashyap** | **Asset Research, Collection & Presentation** – Researched and gathered game assets, including audio and launch vehicle resources, and contributed to preparing the project presentation (PPT). |
| **Harshita Shrivastava** | **Asset Research & Collection** – Assisted in gathering and organizing visual and audio resources for the game. |
| **Harshil Agrawal** | **Presentation & Documentation** – Worked on the project presentation (PPT) and communicating the project concept. |
| **Aryan Kartikey** | **Presentation & Documentation** – Contributed to preparing the project presentation (PPT). |

### Acknowledgements

- **NASA Space Apps Challenge 2026** – For providing the challenge and platform to explore innovative solutions through technology and creativity.
- **SDL Project** – For providing the multimedia libraries used in game development.
- **Open-source community** – For the tools, libraries, and resources that supported the project.

### Third-Party Assets

External images, fonts, audio, and other resources used in MissionCraft belong to their respective creators. Appropriate credits and licence information should be provided wherever applicable.

---

**MissionCraft — Design. Engineer. Explore.**

*Developed for NASA Space Apps Challenge 2026.*

*Built with C++17 and SDL3.*
