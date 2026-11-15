
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
	bash -lc "rm -rf build && mkdir -p build/saves"
	$(WINDRES) resources.rc -O coff -o $(BUILD_DIR)/resources.o
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $(SRC) $(BUILD_DIR)/resources.o -o $(BUILD_DIR)/MissionCraft.exe $(LIBS)
	bash -lc "cp -r assets build/ && if [ -d config ]; then cp -r config build/; fi && ldd build/MissionCraft.exe | awk '\$$3 ~ /\/(ucrt64|mingw64)\/bin\/.*\.dll$$/ {print \$$3}' | sort -u | while read dll; do cp \"\$$dll\" build/; done"
	bash -lc "printf 'MissionCraft - Windows Release\n\nExtract this folder and run MissionCraft.exe.\nDo not separate the executable from its assets or DLL files.\n' > build/README.txt"

clean:
	del /Q $(TARGET) $(RESOURCE) 2>NUL
	if exist build rmdir /S /Q build

.PHONY: all run build clean
