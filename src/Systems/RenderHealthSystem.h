#pragma once

#include "../AssetStore/AssetStore.h"
#include "../Components/HealthComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <string>

class RenderHealthSystem : public System {
public:
    RenderHealthSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
        RequireComponent<HealthComponent>();
    }

    void Update(
        SDL_Renderer *renderer, AssetStore &assetStore, const SDL_Rect &camera
    ) {
        for (const auto &entity : GetSystemEntities()) {
            const auto &transform = entity.GetComponent<TransformComponent>();
            const auto &sprite = entity.GetComponent<SpriteComponent>();
            const auto &health = entity.GetComponent<HealthComponent>();
            const auto &font = assetStore.GetFont("charriot-font");

            const int cameraOffsetX = sprite.isFixed ? 0 : camera.x;
            const int cameraOffsetY = sprite.isFixed ? 0 : camera.y;
            const int spriteWidth = sprite.width * transform.scale.x;
            const int spriteHeight = sprite.height * transform.scale.x;

            const std::string healthText =
                std::to_string(health.healthPercentage) + "%";
            float healthCoeff =
                std::clamp(health.healthPercentage, 0, 100) / 100.f;
            SDL_Color color = {
                static_cast<Uint8>(255 * (1 - healthCoeff)),
                static_cast<Uint8>(255 * healthCoeff),
                0,
            };

            SDL_Surface *surface =
                TTF_RenderText_Blended(font, healthText.c_str(), color);
            SDL_Texture *texture =
                SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            const int healthTextWidth = 20;
            const int healthTextHeight = 10;
            const int healthBarWidth = healthTextWidth;
            const int healthBarHeight = 5;

            SDL_Rect textDstRect = {
                static_cast<int>(
                    transform.position.x + spriteWidth - cameraOffsetX
                ),
                static_cast<int>(
                    transform.position.y + spriteHeight / 2.f - healthBarHeight - healthTextHeight -
                    cameraOffsetY
                ),
                healthTextWidth,
                healthTextHeight,
            };
            SDL_RenderCopy(renderer, texture, NULL, &textDstRect);

            SDL_Rect barDstRect = {
                static_cast<int>(
                    transform.position.x + spriteWidth - cameraOffsetX
                ),
                static_cast<int>(
                    transform.position.y + spriteHeight / 2.f - healthBarHeight - cameraOffsetY
                ),
                static_cast<int>(healthBarWidth * healthCoeff),
                healthBarHeight
            };
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0);
            SDL_RenderFillRect(renderer, &barDstRect);
            SDL_DestroyTexture(texture);
        }
    }
};
