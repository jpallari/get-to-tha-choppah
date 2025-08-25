#pragma once

#include "../Components/BoxColliderComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>

class RenderGUISystem : public System {
public:
    RenderGUISystem() = default;

    void Update(SDL_Renderer *renderer, Registry &registry) {
        // Prelude
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Spawn enemies")) {
            static int enemyPosX = 100, enemyPosY = 100;
            static int enemyScaleX = 1, enemyScaleY = 1;
            static float enemyRotation = 0;
            static int enemyVelocityX = 0, enemyVelocityY = 0;
            static int enemyHealth = 100;
            static int enemyProjectileSpeed = 1;
            static float enemyProjectileAngleDeg = 0;
            static int enemyProjectileRepeatFreq = 1000,
                       enemyProjectileDuration = 1000;
            static std::vector<std::string> sprites{"tank", "truck"};
            static unsigned int selectedSpriteIndex = 0;

            ImGui::Text("Position");
            ImGui::InputInt("X##enemyPos", &enemyPosX);
            ImGui::InputInt("Y##enemyPos", &enemyPosY);
            ImGui::Text("Scale");
            ImGui::SliderInt("X##enemyScale", &enemyScaleX, 1, 10);
            ImGui::SliderInt("Y##enemyScale", &enemyScaleY, 1, 10);
            ImGui::Text("Rotation");
            ImGui::SliderAngle("Rotation", &enemyRotation, 0, 360);
            ImGui::Text("Velocity");
            ImGui::InputInt("X##enemyVelocity", &enemyVelocityX);
            ImGui::InputInt("Y##enemyVelocity", &enemyVelocityY);
            ImGui::Text("Health");
            ImGui::SliderInt("Health", &enemyHealth, 1, 100);
            ImGui::Text("Projectile");
            ImGui::SliderInt("Speed", &enemyProjectileSpeed, 1, 500);
            ImGui::SliderAngle("Angle", &enemyProjectileAngleDeg, 0, 360);
            ImGui::SliderInt(
                "Repeat frequency",
                &enemyProjectileRepeatFreq,
                1 * 1000,
                20 * 1000
            );
            ImGui::SliderInt(
                "Duration", &enemyProjectileDuration, 1 * 1000, 10 * 1000
            );
            ImGui::Text("Sprite");
            if (ImGui::BeginCombo("Sprite", sprites[selectedSpriteIndex].c_str())) {
                for (unsigned int i = 0; i < sprites.size(); i += 1) {
                    const bool isSelected = selectedSpriteIndex == i;
                    if (ImGui::Selectable(sprites[i].c_str(), isSelected)) {
                        selectedSpriteIndex = i;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Create")) {
                Entity enemy = registry.CreateEntity();
                enemy.Group("enemies");
                enemy.AddComponent<TransformComponent>(
                    glm::vec2(enemyPosX, enemyPosY),
                    glm::vec2(enemyScaleX, enemyScaleY),
                    glm::degrees(enemyRotation)
                );
                enemy.AddComponent<RigidBodyComponent>(
                    glm::vec2(enemyVelocityX, enemyVelocityY)
                );
                std::string assetId = sprites[selectedSpriteIndex] + "-image";
                enemy.AddComponent<SpriteComponent>(assetId, 32, 32, 1);
                enemy.AddComponent<BoxColliderComponent>(32, 32);
                enemy.AddComponent<ProjectileEmitterComponent>(
                    glm::vec2(
                        cos(enemyProjectileAngleDeg) * enemyProjectileSpeed,
                        sin(enemyProjectileAngleDeg) * enemyProjectileSpeed
                    ),
                    enemyProjectileRepeatFreq,
                    enemyProjectileDuration,
                    10,
                    false
                );
                enemy.AddComponent<HealthComponent>(enemyHealth);
            }
        }

        // Finale
        ImGui::End();
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    }
};
