#include "Game.h"
#include "Components/AnimationComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/CameraFollowComponent.h"
#include "Components/HealthComponent.h"
#include "Components/KeyboardControlledComponent.h"
#include "Components/ProjectileEmitterComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TextLabelComponent.h"
#include "Components/TransformComponent.h"
#include "Events/EventBus.h"
#include "Events/KeyPressedEvent.h"
#include "Logger.h"
#include "Systems/AnimationSystem.h"
#include "Systems/CameraMovementSystem.h"
#include "Systems/CollisionSystem.h"
#include "Systems/DamageSystem.h"
#include "Systems/KeyboardControlSystem.h"
#include "Systems/MovementSystem.h"
#include "Systems/ProjectileEmitSystem.h"
#include "Systems/ProjectileLifecycleSystem.h"
#include "Systems/RenderColliderSystem.h"
#include "Systems/RenderGUISystem.h"
#include "Systems/RenderHealthSystem.h"
#include "Systems/RenderSystem.h"
#include "Systems/RenderTextSystem.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <fstream>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include <iostream>
#include <sstream>
#include <string>

int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;

Game::Game() {
    isRunning = false;
    isDebug = false;
    registry = std::make_unique<Registry>();
    assetStore = std::make_unique<AssetStore>();
    eventBus = std::make_unique<EventBus>();
    Logger::Log("Game created");
}

Game::~Game() { Logger::Log("Game destroyed"); }

void Game::Initialize() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Logger::Err("Error initializing SDL.");
        return;
    }
    if (TTF_Init() != 0) {
        Logger::Err("Error initializing SDL TTF.");
        return;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    windowWidth = displayMode.w;  // 800
    windowHeight = displayMode.h; // 600

    window = SDL_CreateWindow(
        "2dgameengine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!window) {
        Logger::Err("Error creating SDL window.");
        return;
    }
    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        Logger::Err("Error creating SDL renderer.");
        return;
    }
    SDL_RenderSetLogicalSize(renderer, windowWidth, windowHeight);
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    camera.x = 0;
    camera.y = 0;
    camera.w = windowWidth;
    camera.h = windowHeight;

    isRunning = true;
}

void Game::LoadLevel(int level) {
    registry->AddSystem<AnimationSystem>();
    registry->AddSystem<MovementSystem>();
    registry->AddSystem<RenderSystem>();
    registry->AddSystem<CollisionSystem>();
    registry->AddSystem<RenderColliderSystem>();
    registry->AddSystem<DamageSystem>();
    registry->AddSystem<KeyboardControlSystem>();
    registry->AddSystem<CameraMovementSystem>();
    registry->AddSystem<ProjectileEmitSystem>();
    registry->AddSystem<ProjectileLifecycleSystem>();
    registry->AddSystem<RenderTextSystem>();
    registry->AddSystem<RenderHealthSystem>();
    registry->AddSystem<RenderGUISystem>();

    assetStore->AddTexture(
        renderer, "chopper-image", "./assets/images/chopper-spritesheet.png"
    );
    assetStore->AddTexture(
        renderer, "radar-image", "./assets/images/radar.png"
    );
    assetStore->AddTexture(
        renderer, "tank-image", "./assets/images/tank-panther-right.png"
    );
    assetStore->AddTexture(
        renderer, "truck-image", "./assets/images/truck-ford-right.png"
    );
    assetStore->AddTexture(
        renderer, "tree-image", "./assets/images/tree.png"
    );
    assetStore->AddTexture(
        renderer, "tilemap-image", "./assets/tilemaps/jungle.png"
    );
    assetStore->AddTexture(
        renderer, "bullet-image", "./assets/images/bullet.png"
    );
    assetStore->AddFont("charriot-font", "./assets/fonts/charriot.ttf", 20);

    {
        std::string mapLayout;
        {
            std::ifstream file("./assets/tilemaps/jungle.map");
            file.seekg(0, std::ios::end);
            mapLayout.reserve(file.tellg());
            file.seekg(0, std::ios::beg);
            mapLayout.assign(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>()
            );
        }
        std::stringstream mapLayoutStream(mapLayout);

        const int tileSize = 32;
        const int tileScale = 2;
        const int columns = 25;
        const int rows = 20;
        int tileX = -1, tileY = -1;

        for (int row = 0; row < rows; row++) {
            for (int column = 0; column < columns; column++) {
                char c = mapLayoutStream.get();
                tileY = c - '0';
                c = mapLayoutStream.get();
                tileX = c - '0';
                mapLayoutStream.ignore();
                Entity tile = registry->CreateEntity();
                tile.Group("tiles");
                tile.AddComponent<TransformComponent>(
                    glm::vec2(
                        tileScale * tileSize * column,
                        tileScale * tileSize * row
                    ),
                    glm::vec2(tileScale, tileScale)
                );
                tile.AddComponent<SpriteComponent>(
                    "tilemap-image",
                    tileSize,
                    tileSize,
                    0,
                    false,
                    tileSize * tileX,
                    tileSize * tileY
                );
            }
        }
        mapWidth = columns * tileSize * tileScale;
        mapHeight = rows * tileSize * tileScale;
    }

    Entity chopper = registry->CreateEntity();
    chopper.Tag("player");
    chopper.AddComponent<TransformComponent>(
        glm::vec2(240.0, 110.0), glm::vec2(1.0, 1.0), 0.0
    );
    chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 2);
    chopper.AddComponent<AnimationComponent>(2, 15);
    chopper.AddComponent<BoxColliderComponent>(32, 32);
    chopper.AddComponent<KeyboardControlledComponent>(
        glm::vec2(0, -80), glm::vec2(80, 0), glm::vec2(0, 80), glm::vec2(-80, 0)
    );
    chopper.AddComponent<CameraFollowComponent>();
    chopper.AddComponent<HealthComponent>(100);
    chopper.AddComponent<ProjectileEmitterComponent>(
        glm::vec2(150.0, 150.0), 200, 3000, 10, true
    );

    Entity radar = registry->CreateEntity();
    radar.AddComponent<TransformComponent>(
        glm::vec2(windowWidth - 74.0, 10), glm::vec2(1.0, 1.0), 0.0
    );
    radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 2, true);
    radar.AddComponent<AnimationComponent>(8, 5);

    Entity tank = registry->CreateEntity();
    tank.Group("enemies");
    tank.AddComponent<TransformComponent>(
        glm::vec2(500.0, 500.0), glm::vec2(1.0, 1.0), 0.0
    );
    tank.AddComponent<RigidBodyComponent>(glm::vec2(20.0, 0.0));
    tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 1);
    tank.AddComponent<BoxColliderComponent>(32, 32);
    tank.AddComponent<ProjectileEmitterComponent>(
        glm::vec2(100.0, 0.0), 5000, 3000, 10, false
    );
    tank.AddComponent<HealthComponent>(100);

    Entity truck = registry->CreateEntity();
    truck.Group("enemies");
    truck.AddComponent<TransformComponent>(
        glm::vec2(120.0, 500.0), glm::vec2(1.0, 1.0), 0.0
    );
    truck.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 2);
    truck.AddComponent<BoxColliderComponent>(32, 32);
    truck.AddComponent<ProjectileEmitterComponent>(
        glm::vec2(0.0, -100.0), 2000, 5000, 10, false
    );
    truck.AddComponent<HealthComponent>(100);

    Entity treeA = registry->CreateEntity();
    treeA.Group("obstacles");
    treeA.AddComponent<TransformComponent>(
        glm::vec2(600.0, 495.0), glm::vec2(1.0, 1.0), 0.0
    );
    treeA.AddComponent<SpriteComponent>("tree-image", 16, 32, 2);
    treeA.AddComponent<BoxColliderComponent>(16, 32);

    Entity treeB = registry->CreateEntity();
    treeB.Group("obstacles");
    treeB.AddComponent<TransformComponent>(
        glm::vec2(400.0, 495.0), glm::vec2(1.0, 1.0), 0.0
    );
    treeB.AddComponent<SpriteComponent>("tree-image", 16, 32, 2);
    treeB.AddComponent<BoxColliderComponent>(16, 32);

    Entity gameName = registry->CreateEntity();
    SDL_Color green = {0, 255, 0};
    gameName.AddComponent<TextLabelComponent>(
        glm::vec2(windowWidth / 2 - 100, 10),
        "Get to tha choppah! v0.0001",
        "charriot-font",
        green,
        true
    );
}

void Game::Setup() { LoadLevel(1); }

void Game::Update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - msPreviousFrame);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
        SDL_Delay(timeToWait);
    }
    double deltaTime = (SDL_GetTicks() - msPreviousFrame) / 1000.0;
    msPreviousFrame = SDL_GetTicks();

    // add subscriptions
    eventBus->Reset();
    registry->GetSystem<MovementSystem>().SubscribeToEvents(*eventBus);
    registry->GetSystem<DamageSystem>().SubscribeToEvents(*eventBus);
    registry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(*eventBus);
    registry->GetSystem<ProjectileEmitSystem>().SubscribeToEvents(*eventBus);

    // update systems
    registry->Update();
    registry->GetSystem<MovementSystem>().Update(deltaTime);
    registry->GetSystem<AnimationSystem>().Update();
    registry->GetSystem<CollisionSystem>().Update(*eventBus);
    registry->GetSystem<CameraMovementSystem>().Update(camera);
    registry->GetSystem<ProjectileEmitSystem>().Update(*registry);
    registry->GetSystem<ProjectileLifecycleSystem>().Update();
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);
    registry->GetSystem<RenderSystem>().Update(renderer, *assetStore, camera);
    registry->GetSystem<RenderHealthSystem>().Update(
        renderer, *assetStore, camera
    );
    registry->GetSystem<RenderTextSystem>().Update(
        renderer, *assetStore, camera
    );
    if (isDebug) {
        registry->GetSystem<RenderColliderSystem>().Update(renderer, camera);
        registry->GetSystem<RenderGUISystem>().Update(renderer, *registry);
    }
    SDL_RenderPresent(renderer);
}

void Game::Run() {
    Setup();
    while (isRunning) {
        ProcessInput();
        Update();
        Render();
    }
}

void Game::ProcessInput() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        ImGui_ImplSDL2_ProcessEvent(&sdlEvent);

        switch (sdlEvent.type) {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            switch (sdlEvent.key.keysym.sym) {
            case SDLK_ESCAPE:
                isRunning = false;
                break;
            case SDLK_d:
                isDebug = !isDebug;
                break;
            }
            eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);
            break;
        }
    }
}

void Game::Destroy() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
