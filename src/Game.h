#pragma once

#include "AssetStore/AssetStore.h"
#include "ECS/ECS.h"
#include "Events/EventBus.h"
#include <SDL2/SDL.h>
#include <memory>

constexpr int FPS = 400;
constexpr int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
private:
    bool isRunning = false;
    bool isDebug = false;
    int msPreviousFrame = 0;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Rect camera;
    std::unique_ptr<Registry> registry;
    std::unique_ptr<AssetStore> assetStore;
    std::unique_ptr<EventBus> eventBus;

public:
    Game();
    ~Game();
    void Initialize();
    void Run();
    void LoadLevel(int level);
    void Setup();
    void ProcessInput();
    void Update();
    void Render();
    void Destroy();

    static int windowWidth;
    static int windowHeight;
    static int mapWidth;
    static int mapHeight;
};
