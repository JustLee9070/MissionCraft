
CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra -I src/include -I src
LDFLAGS = -L src/lib
LIBS = -lmingw32 -lSDL3 -lSDL3_image

SRC = main.cpp src/Game.cpp
TARGET = MissionCraft.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET) $(LIBS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
