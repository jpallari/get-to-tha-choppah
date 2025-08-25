#pragma once

#include <SDL2/SDL.h>

struct ProjectileComponent {
    bool isFriendly;
    int hitPercentageDamage;
    unsigned int duration;
    unsigned int startTime;

    ProjectileComponent(
        bool isFriendly = false,
        int hitPercentageDamage = 0,
        unsigned int duration = 0
    )
    : isFriendly(isFriendly),
      hitPercentageDamage(hitPercentageDamage),
      duration(duration) {
        startTime = SDL_GetTicks();
    }
};
