#pragma once

#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include "../Events/CollisionEvent.h"
#include "../Events/EventBus.h"

class CollisionSystem : public System {
public:
    CollisionSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<BoxColliderComponent>();
    }

    void Update(EventBus &eventBus) {
        auto &entities = GetSystemEntities();
        for (auto i = entities.begin(); i != entities.end(); i++) {
            Entity a = *i;
            auto &aTransform = a.GetComponent<TransformComponent>();
            auto &aCollider = a.GetComponent<BoxColliderComponent>();

            for (auto j = i; j != entities.end(); j++) {
                Entity b = *j;
                if (a == b) {
                    continue;
                }

                auto &bTransform = b.GetComponent<TransformComponent>();
                auto &bCollider = b.GetComponent<BoxColliderComponent>();
                bool collided = AABBCollision(
                    aTransform.position.x + aCollider.offset.x,
                    aTransform.position.y + aCollider.offset.y,
                    aCollider.width,
                    aCollider.height,
                    bTransform.position.x + bCollider.offset.x,
                    bTransform.position.y + bCollider.offset.y,
                    bCollider.width,
                    bCollider.height
                );
                if (collided) {
                    eventBus.EmitEvent<CollisionEvent>(a, b);
                }
            }
        }
    }

private:
    bool AABBCollision(
        double aX,
        double aY,
        double aW,
        double aH,
        double bX,
        double bY,
        double bW,
        double bH
    ) {
        return aX < bX + bW && aX + aW > bX && aY < bY + bH && aY + aH > bY;
    }
};
