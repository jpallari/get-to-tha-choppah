#pragma once

#include "../AssetStore/AssetStore.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <algorithm>
#include <vector>

class RenderColliderSystem : public System {
public:
    RenderColliderSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
        RequireComponent<BoxColliderComponent>();
    }

    void Update(SDL_Renderer *renderer, const SDL_Rect &camera) {
        for (auto &entity : GetSystemEntities()) {
            const auto &transform = entity.GetComponent<TransformComponent>();
            const auto &collider = entity.GetComponent<BoxColliderComponent>();
            SDL_Rect colliderRect = {
                static_cast<int>(
                    transform.position.x + collider.offset.x - camera.x
                ),
                static_cast<int>(
                    transform.position.y + collider.offset.y - camera.y
                ),
                static_cast<int>(collider.width * transform.scale.x),
                static_cast<int>(collider.height * transform.scale.y)
            };
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &colliderRect);
        }
    }
};
