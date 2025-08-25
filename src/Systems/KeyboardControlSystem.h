#pragma once

#include <SDL2/SDL.h>
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../ECS/ECS.h"
#include "../Events/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Logger.h"
#include <string>

class KeyboardControlSystem : public System {
public:
    KeyboardControlSystem() {
        RequireComponent<KeyboardControlledComponent>();
        RequireComponent<SpriteComponent>();
        RequireComponent<RigidBodyComponent>();
    }

    void SubscribeToEvents(EventBus &eventBus) {
        eventBus.SubscribeToEvent<KeyPressedEvent>(
            this, &KeyboardControlSystem::OnKeyPressed
        );
    }

    void OnKeyPressed(KeyPressedEvent &event) {
        for (auto &entity : GetSystemEntities()) {
            const auto &keyboard = entity.GetComponent<KeyboardControlledComponent>();
            auto &sprite = entity.GetComponent<SpriteComponent>();
            auto &body = entity.GetComponent<RigidBodyComponent>();

            switch (event.key) {
            case SDLK_UP:
                body.velocity = keyboard.upVelocity;
                sprite.srcRect.y = sprite.height * 0;
                break;
            case SDLK_RIGHT:
                body.velocity = keyboard.rightVelocity;
                sprite.srcRect.y = sprite.height * 1;
                break;
            case SDLK_DOWN:
                body.velocity = keyboard.downVelocity;
                sprite.srcRect.y = sprite.height * 2;
                break;
            case SDLK_LEFT:
                body.velocity = keyboard.leftVelocity;
                sprite.srcRect.y = sprite.height * 3;
                break;
            }
        }
    }

    void Update() {}
};
