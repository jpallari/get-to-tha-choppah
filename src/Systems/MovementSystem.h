#pragma once

#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include "../Events/CollisionEvent.h"
#include "../Events/EventBus.h"
#include "../Game.h"
#include "../Logger.h"
#include <SDL2/SDL_render.h>
#include <algorithm>

class MovementSystem : public System {
public:
    MovementSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<RigidBodyComponent>();
    }

    void SubscribeToEvents(EventBus &eventBus) {
        eventBus.SubscribeToEvent<CollisionEvent>(
            this, &MovementSystem::OnCollision
        );
    }

    void Update(double deltaTime) {
        for (auto entity : GetSystemEntities()) {
            auto &transform = entity.GetComponent<TransformComponent>();
            const auto &rigidBody = entity.GetComponent<RigidBodyComponent>();

            transform.position.x += rigidBody.velocity.x * deltaTime;
            transform.position.y += rigidBody.velocity.y * deltaTime;

            bool isEntityOutsideMap =
                (transform.position.x < 0 ||
                 transform.position.x > Game::mapWidth ||
                 transform.position.y < 0 ||
                 transform.position.y > Game::mapHeight);
            if (isEntityOutsideMap) {
                if (entity.HasTag("player")) {
                    transform.position.x = std::clamp(
                        transform.position.x,
                        0.0f,
                        static_cast<float>(Game::mapWidth)
                    );
                    transform.position.y = std::clamp(
                        transform.position.y,
                        0.0f,
                        static_cast<float>(Game::mapHeight)
                    );
                } else {
                    entity.Kill();
                }
            }
        }
    }

    void OnCollision(CollisionEvent &event) {
        Entity &a = event.a;
        Entity &b = event.b;

        if (a.BelongsToGroup("enemies") && b.BelongsToGroup("obstacles")) {
            Logger::Log("Bounce! a -> b");
            OnEnemyHitsObstacle(a, b);
        }
        if (a.BelongsToGroup("obstacles") && b.BelongsToGroup("enemies")) {
            Logger::Log("Bounce! b -> a");
            OnEnemyHitsObstacle(b, a);
        }
    }

    void OnEnemyHitsObstacle(Entity &enemy, Entity &obstacle) {
        if (enemy.HasComponent<RigidBodyComponent>()) {
            auto &rigidBody = enemy.GetComponent<RigidBodyComponent>();
            bool flipVelocityX = rigidBody.velocity.x != 0;
            bool flipVelocityY = rigidBody.velocity.y != 0;

            if (flipVelocityX) {
                rigidBody.velocity.x *= -1;
            }
            if (flipVelocityY) {
                rigidBody.velocity.y *= -1;
            }

            if (enemy.HasComponent<SpriteComponent>()) {
                auto &sprite = enemy.GetComponent<SpriteComponent>();
                if (flipVelocityX) {
                    sprite.flip = (sprite.flip == SDL_FLIP_NONE)
                                      ? SDL_FLIP_HORIZONTAL
                                      : SDL_FLIP_NONE;
                }
            }
            if (enemy.HasComponent<ProjectileEmitterComponent>()) {
                auto &emitter =
                    enemy.GetComponent<ProjectileEmitterComponent>();
                if (flipVelocityX) {
                    emitter.projectileVelocity.x *= -1;
                }
                if (flipVelocityY) {
                    emitter.projectileVelocity.y *= -1;
                }
            }
        }
    }
};
