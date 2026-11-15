
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
	if exist build rmdir /S /Q build
	mkdir build
	mkdir build\saves
	$(WINDRES) resources.rc -O coff -o build\resources.o
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $(SRC) build\resources.o -o build\MissionCraft.exe $(LIBS)
	powershell -NoProfile -ExecutionPolicy Bypass -Command "Copy-Item assets build -Recurse"
	powershell -NoProfile -ExecutionPolicy Bypass -Command "if (Test-Path config) { Copy-Item config build -Recurse }"
	powershell -NoProfile -ExecutionPolicy Bypass -Command "$$deps = & objdump -p build/MissionCraft.exe | Select-String 'DLL Name:'; foreach ($$dep in $$deps) { $$name = ($$dep.ToString() -split ':')[1].Trim(); $$path = & where.exe $$name 2>$$null | Select-Object -First 1; if ($$path) { Copy-Item $$path build -Force } }"
	powershell -NoProfile -ExecutionPolicy Bypass -Command "Set-Content build/README.txt 'MissionCraft - Windows Release`n`nExtract this folder and run MissionCraft.exe.`nKeep the executable, assets, and DLL files together.'"

clean:
	del /Q $(TARGET) $(RESOURCE) 2>NUL
	if exist build rmdir /S /Q build

.PHONY: all run build clean
