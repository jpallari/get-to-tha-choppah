#pragma once

#include "../AssetStore/AssetStore.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <algorithm>
#include <vector>

struct RenderableEntity {
    TransformComponent transformComponent;
    SpriteComponent spriteComponent;
};

class RenderSystem : public System {
public:
    RenderSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
    }

    void Update(
        SDL_Renderer *renderer, AssetStore &assetStore, const SDL_Rect &camera
    ) {
        std::vector<RenderableEntity> renderableEntities;
        for (auto &entity : GetSystemEntities()) {
            const auto &t = entity.GetComponent<TransformComponent>();
            const auto &s = entity.GetComponent<SpriteComponent>();

            bool isEntityOutsideCameraView =
                (t.position.x + t.scale.x * s.width < camera.x ||
                 t.position.x > camera.x + camera.w ||
                 t.position.y + t.scale.y * s.height < camera.y ||
                 t.position.y > camera.y + camera.h);
            if (isEntityOutsideCameraView && !s.isFixed) {
                continue;
            }

            RenderableEntity renderableEntity{
                t,
                s,
            };
            renderableEntities.emplace_back(renderableEntity);
        }

        std::sort(
            renderableEntities.begin(),
            renderableEntities.end(),
            [](RenderableEntity a, RenderableEntity b) {
                return a.spriteComponent.zIndex < b.spriteComponent.zIndex;
            }
        );

        for (auto &entity : renderableEntities) {
            const auto &transform = entity.transformComponent;
            const auto &sprite = entity.spriteComponent;

            const int cameraOffsetX = sprite.isFixed ? 0 : camera.x;
            const int cameraOffsetY = sprite.isFixed ? 0 : camera.y;
            const int spriteWidth = sprite.width * transform.scale.x;
            const int spriteHeight = sprite.height * transform.scale.x;

            SDL_Rect dstRect = {
                static_cast<int>(transform.position.x - cameraOffsetX),
                static_cast<int>(transform.position.y - cameraOffsetY),
                spriteWidth,
                spriteHeight
            };
            SDL_RenderCopyEx(
                renderer,
                assetStore.GetTexture(sprite.assetId),
                &sprite.srcRect,
                &dstRect,
                transform.rotation,
                NULL,
                sprite.flip
            );
        }
    }
};
