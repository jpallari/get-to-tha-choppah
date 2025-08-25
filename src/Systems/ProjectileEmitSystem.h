#pragma once

#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include "../Events/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

class ProjectileEmitSystem : public System {
public:
    ProjectileEmitSystem() {
        RequireComponent<ProjectileEmitterComponent>();
        RequireComponent<TransformComponent>();
    }

    void SubscribeToEvents(EventBus &eventBus) {
        eventBus.SubscribeToEvent<KeyPressedEvent>(
            this, &ProjectileEmitSystem::OnKeyPress
        );
    }

    void OnKeyPress(KeyPressedEvent &event) {
        if (event.key != SDLK_SPACE) {
            return;
        }

        for (auto &entity : GetSystemEntities()) {
            auto &projectileEmitter =
                entity.GetComponent<ProjectileEmitterComponent>();
            const auto &transform = entity.GetComponent<TransformComponent>();

            if (!projectileEmitter.isFriendly) {
                continue;
            }

            if (SDL_GetTicks() - projectileEmitter.lastEmissionTime >
                projectileEmitter.repeatFrequency) {
                glm::vec2 projectilePosition = transform.position;
                if (entity.HasComponent<SpriteComponent>()) {
                    const auto &sprite = entity.GetComponent<SpriteComponent>();
                    projectilePosition.x +=
                        transform.scale.x * sprite.width / 2;
                    projectilePosition.y +=
                        transform.scale.y * sprite.height / 2;
                }

                Entity projectile = entity.registry->CreateEntity();
                projectile.Group("projectiles");
                projectile.AddComponent<TransformComponent>(
                    projectilePosition, glm::vec2(2.0, 2.0), 0.0
                );
                auto projectileVelocity = projectileEmitter.projectileVelocity;
                if (entity.HasComponent<RigidBodyComponent>()) {
                    const auto &rigidBody =
                        entity.GetComponent<RigidBodyComponent>();
                    int directionX = 0;
                    int directionY = 0;
                    if (rigidBody.velocity.x > 0) { directionX = +1; }
                    if (rigidBody.velocity.x < 0) { directionX = -1; }
                    if (rigidBody.velocity.y > 0) { directionY = +1; }
                    if (rigidBody.velocity.y < 0) { directionY = -1; }

                    projectileVelocity.x *= directionX;
                    projectileVelocity.y *= directionY;
                }

                projectile.AddComponent<RigidBodyComponent>(projectileVelocity);
                projectile.AddComponent<SpriteComponent>(
                    "bullet-image", 4, 4, 4
                );
                projectile.AddComponent<BoxColliderComponent>(4, 4);
                projectile.AddComponent<ProjectileComponent>(
                    projectileEmitter.isFriendly,
                    projectileEmitter.hitPercentDamage,
                    projectileEmitter.projectileDuration
                );
                projectileEmitter.lastEmissionTime = SDL_GetTicks();
            }
        }
    }

    void Update(Registry &registry) {
        for (auto &entity : GetSystemEntities()) {
            auto &projectileEmitter =
                entity.GetComponent<ProjectileEmitterComponent>();
            const auto &transform = entity.GetComponent<TransformComponent>();

            if (projectileEmitter.repeatFrequency == 0 ||
                projectileEmitter.isFriendly) {
                continue;
            }

            if (SDL_GetTicks() - projectileEmitter.lastEmissionTime >
                projectileEmitter.repeatFrequency) {
                glm::vec2 projectilePosition = transform.position;
                if (entity.HasComponent<SpriteComponent>()) {
                    const auto &sprite = entity.GetComponent<SpriteComponent>();
                    projectilePosition.x +=
                        transform.scale.x * sprite.width / 2;
                    projectilePosition.y +=
                        transform.scale.y * sprite.height / 2;
                }

                Entity projectile = registry.CreateEntity();
                projectile.Group("projectiles");
                projectile.AddComponent<TransformComponent>(
                    projectilePosition, glm::vec2(2.0, 2.0), 0.0
                );
                projectile.AddComponent<RigidBodyComponent>(
                    projectileEmitter.projectileVelocity
                );
                projectile.AddComponent<SpriteComponent>(
                    "bullet-image", 4, 4, 4
                );
                projectile.AddComponent<BoxColliderComponent>(4, 4);
                projectile.AddComponent<ProjectileComponent>(
                    projectileEmitter.isFriendly,
                    projectileEmitter.hitPercentDamage,
                    projectileEmitter.projectileDuration
                );
                projectileEmitter.lastEmissionTime = SDL_GetTicks();
            }
        }
    }
};
