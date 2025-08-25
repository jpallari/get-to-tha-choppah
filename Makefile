CC = g++
LANG_STD = c++17
CFLAGS = -Wall -Wfatal-errors -g
INCLUDE_PATH = -I"./libs"
SRC_FILES = src/*.cpp \
			src/ECS/*.cpp \
			src/AssetStore/*.cpp \
			libs/imgui/*.cpp
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua 
OBJ_NAME = gameengine

build:
	$(CC) $(CFLAGS) -std=$(LANG_STD) $(INCLUDE_PATH) $(SRC_FILES) $(LDFLAGS) -o $(OBJ_NAME)

run:
	./$(OBJ_NAME)

clean:
	rm $(OBJ_NAME)

.PHONY: clean build run
