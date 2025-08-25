#pragma once

#include "Event.h"
#include <SDL2/SDL.h>

class KeyPressedEvent : public Event {
public:
    SDL_Keycode key;

    KeyPressedEvent(SDL_Keycode key)
    : key(key) {}
};
