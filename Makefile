
CXX = g++
WINDRES = windres

CXXFLAGS = -std=c++17 -Wall -Wextra -I src/include -I src/external
RELEASE_FLAGS = -O2 -DNDEBUG

LIBS = -lSDL3 -lSDL3_image -lSDL3_ttf -lSDL3_mixer

SRC = main.cpp src/Game.cpp src/OptionsManager.cpp src/GameState.cpp \
      src/LaunchVehicle.cpp \
      src/Component.cpp src/Spacecraft.cpp src/Destination.cpp src/AssetManager.cpp src/SaveManager.cpp src/GameAudio.cpp \
      src/screens/TitleScreen.cpp \
      src/screens/MainMenu.cpp \
      src/screens/DesignerScreen.cpp \
      src/screens/PlannerScreen.cpp \
      src/screens/MissionControl.cpp \
      src/screens/OptionsScreen.cpp

TARGET = MissionCraft.exe
RESOURCE = resources.o
BUILD_DIR = build

all: $(TARGET)

$(RESOURCE): resources.rc assets/icons/missioncraft.ico
	$(WINDRES) resources.rc -O coff -o $(RESOURCE)

$(TARGET): $(SRC) $(RESOURCE)
	$(CXX) $(CXXFLAGS) $(SRC) $(RESOURCE) -o $(TARGET) $(LIBS)

run: all
	.\$(TARGET)

build:
	if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)
	mkdir $(BUILD_DIR)
	mkdir $(BUILD_DIR)\saves
	$(WINDRES) resources.rc -O coff -o $(BUILD_DIR)\resources.o
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $(SRC) $(BUILD_DIR)\resources.o -o $(BUILD_DIR)\MissionCraft.exe $(LIBS)
	powershell -NoProfile -ExecutionPolicy Bypass -Command "Copy-Item assets $(BUILD_DIR) -Recurse"
	powershell -NoProfile -ExecutionPolicy Bypass -Command "if (Test-Path config) { Copy-Item config $(BUILD_DIR) -Recurse }"
	powershell -NoProfile -ExecutionPolicy Bypass -Command "$$bin = Split-Path (Get-Command g++.exe).Source -Parent; $$objdump = Join-Path $$bin 'objdump.exe'; $$queue = [System.Collections.Generic.Queue[string]]::new(); $$queue.Enqueue((Resolve-Path '$(BUILD_DIR)/MissionCraft.exe').Path); $$seen = @{}; while ($$queue.Count -gt 0) { $$file = $$queue.Dequeue(); foreach ($$dep in (& $$objdump -p $$file 2>$$null | Select-String 'DLL Name:')) { $$name = ($$dep.ToString() -split ':', 2)[1].Trim(); if (-not $$seen.ContainsKey($$name)) { $$seen[$$name] = $$true; $$path = Join-Path $$bin $$name; if (Test-Path $$path) { Copy-Item $$path '$(BUILD_DIR)' -Force; $$queue.Enqueue($$path) } } } }"
	powershell -NoProfile -ExecutionPolicy Bypass -Command "Set-Content '$(BUILD_DIR)/README.txt' 'MissionCraft - Windows Release'; Add-Content '$(BUILD_DIR)/README.txt' ''; Add-Content '$(BUILD_DIR)/README.txt' 'Extract this folder and run MissionCraft.exe.'; Add-Content '$(BUILD_DIR)/README.txt' 'Keep the executable, assets, and DLL files together.'"

clean:
	del /Q $(TARGET) $(RESOURCE) 2>NUL
	if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)

.PHONY: all run build clean
