CC = g++
LANG_STD = c++17
CFLAGS = -Wall -Wfatal-errors -g -I"./libs" -std=$(LANG_STD) -MMD -MP
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua
LIBS_FILES = $(wildcard libs/imgui/*.cpp)
LIBS_OBJ_FILES = $(LIBS_FILES:libs/%.cpp=build/%.o)
SRC_FILES = $(wildcard src/*.cpp) \
			$(wildcard src/ECS/*.cpp) \
			$(wildcard src/AssetStore/*.cpp)
OBJ_FILES = $(SRC_FILES:src/%.cpp=build/%.o)
GAME_EXEC_NAME = gameengine

all: gameengine

-include $(OBJ_FILES:.o=.d)
-include $(LIBS_OBJ_FILES:.o=.d)

$(GAME_EXEC_NAME): $(OBJ_FILES) $(LIBS_OBJ_FILES)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_FILES): build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBS_OBJ_FILES): build/%.o: libs/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run:
	./$(GAME_EXEC_NAME)

clean:
	rm -rf $(GAME_EXEC_NAME)
	rm -rf build

.PHONY: clean run
