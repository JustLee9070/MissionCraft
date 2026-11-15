
CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra -I src/include -I src/external

LIBS = -lSDL3 -lSDL3_image -lSDL3_ttf -lSDL3_mixer

SRC = main.cpp src/Game.cpp src/OptionsManager.cpp src/GameState.cpp \
      src/Component.cpp src/Spacecraft.cpp src/Destination.cpp src/AssetManager.cpp src/SaveManager.cpp src/GameAudio.cpp \
      src/screens/TitleScreen.cpp \
      src/screens/MainMenu.cpp \
      src/screens/DesignerScreen.cpp \
      src/screens/PlannerScreen.cpp \
      src/screens/MissionControl.cpp \
      src/screens/OptionsScreen.cpp

TARGET = MissionCraft.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LIBS)

run: all
	.\$(TARGET)

clean:
	del /Q $(TARGET) 2>NUL

.PHONY: all run clean
