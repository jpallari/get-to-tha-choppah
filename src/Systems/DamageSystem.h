#pragma once

#include "../Components/BoxColliderComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../ECS/ECS.h"
#include "../Events/CollisionEvent.h"
#include "../Events/EventBus.h"

class DamageSystem : public System {
public:
    DamageSystem() { RequireComponent<BoxColliderComponent>(); }

    void SubscribeToEvents(EventBus &eventBus) {
        eventBus.SubscribeToEvent<CollisionEvent>(
            this, &DamageSystem::OnCollision
        );
    }

    void OnCollision(CollisionEvent &event) {
        Entity a = event.a;
        Entity b = event.b;

        if (a.BelongsToGroup("projectiles") && b.HasTag("player")) {
            OnProjectileHitsPlayer(a, b);
        }
        if (b.BelongsToGroup("projectiles") && a.HasTag("player")) {
            OnProjectileHitsPlayer(b, a);
        }
        if (a.BelongsToGroup("projectiles") && b.BelongsToGroup("enemies")) {
            OnProjectileHitsEnemy(a, b);
        }
        if (b.BelongsToGroup("projectiles") && a.BelongsToGroup("enemies")) {
            OnProjectileHitsEnemy(b, a);
        }
    }

    void OnProjectileHitsPlayer(Entity projectile, Entity player) {
        auto &projectileComponent =
            projectile.GetComponent<ProjectileComponent>();
        if (projectileComponent.isFriendly) {
            return;
        }

        auto &health = player.GetComponent<HealthComponent>();
        health.healthPercentage -= projectileComponent.hitPercentageDamage;

        if (health.healthPercentage <= 0) {
            player.Kill();
        }

        projectile.Kill();
    }

    void OnProjectileHitsEnemy(Entity projectile, Entity enemy) {
        auto &projectileComponent =
            projectile.GetComponent<ProjectileComponent>();
        if (!projectileComponent.isFriendly) {
            return;
        }

        auto &health = enemy.GetComponent<HealthComponent>();
        health.healthPercentage -= projectileComponent.hitPercentageDamage;

        if (health.healthPercentage <= 0) {
            enemy.Kill();
        }

        projectile.Kill();
    }

    void Update() {}
};
