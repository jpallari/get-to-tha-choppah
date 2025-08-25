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

class RenderSystem : public System {
public:
    RenderSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
    }

    void Update(
        SDL_Renderer *renderer, AssetStore &assetStore, const SDL_Rect &camera
    ) {
        std::vector<Entity> es = GetSystemEntities();
        std::sort(es.begin(), es.end(), [](Entity a, Entity b) {
            const auto &aSprite = a.GetComponent<SpriteComponent>();
            const auto &bSprite = b.GetComponent<SpriteComponent>();
            return aSprite.zIndex < bSprite.zIndex;
        });

        for (auto &entity : es) {
            const auto &transform = entity.GetComponent<TransformComponent>();
            const auto &sprite = entity.GetComponent<SpriteComponent>();

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
