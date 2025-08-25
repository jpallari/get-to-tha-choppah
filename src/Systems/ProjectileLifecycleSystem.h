#pragma once

#include "../Components/ProjectileComponent.h"
#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

class ProjectileLifecycleSystem : public System {
public:
    ProjectileLifecycleSystem() { RequireComponent<ProjectileComponent>(); }

    void Update() {
        for (auto entity : GetSystemEntities()) {
            const auto &projectile = entity.GetComponent<ProjectileComponent>();
            if (SDL_GetTicks() - projectile.startTime > projectile.duration) {
                entity.Kill();
            }
        }
    }
};
