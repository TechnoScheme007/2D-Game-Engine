#pragma once
#include "Math.h"
#include <SDL2/SDL.h>
#include <unordered_map>
#include <string>

namespace Engine {

class Input {
public:
    static Input& instance() {
        static Input inst;
        return inst;
    }

    void update() {
        prevKeys_ = currentKeys_;
        prevMouse_ = currentMouse_;
        mouseWheel_ = 0;
    }

    void handleEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_KEYDOWN:
                if (!event.key.repeat)
                    currentKeys_[event.key.keysym.scancode] = true;
                break;
            case SDL_KEYUP:
                currentKeys_[event.key.keysym.scancode] = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                currentMouse_[event.button.button] = true;
                break;
            case SDL_MOUSEBUTTONUP:
                currentMouse_[event.button.button] = false;
                break;
            case SDL_MOUSEMOTION:
                mousePos_ = {static_cast<float>(event.motion.x),
                             static_cast<float>(event.motion.y)};
                break;
            case SDL_MOUSEWHEEL:
                mouseWheel_ = event.wheel.y;
                break;
        }
    }

    bool isKeyDown(SDL_Scancode key) const {
        auto it = currentKeys_.find(key);
        return it != currentKeys_.end() && it->second;
    }

    bool isKeyPressed(SDL_Scancode key) const {
        bool curr = isKeyDown(key);
        auto it = prevKeys_.find(key);
        bool prev = it != prevKeys_.end() && it->second;
        return curr && !prev;
    }

    bool isKeyReleased(SDL_Scancode key) const {
        bool curr = isKeyDown(key);
        auto it = prevKeys_.find(key);
        bool prev = it != prevKeys_.end() && it->second;
        return !curr && prev;
    }

    bool isMouseDown(uint8_t button) const {
        auto it = currentMouse_.find(button);
        return it != currentMouse_.end() && it->second;
    }

    bool isMousePressed(uint8_t button) const {
        bool curr = isMouseDown(button);
        auto it = prevMouse_.find(button);
        bool prev = it != prevMouse_.end() && it->second;
        return curr && !prev;
    }

    Vec2 getMousePosition() const { return mousePos_; }
    int getMouseWheel() const { return mouseWheel_; }

    // Get axis-style input (-1, 0, 1) for common movement
    float getAxis(const std::string& axis) const {
        if (axis == "horizontal") {
            float val = 0;
            if (isKeyDown(SDL_SCANCODE_A) || isKeyDown(SDL_SCANCODE_LEFT)) val -= 1;
            if (isKeyDown(SDL_SCANCODE_D) || isKeyDown(SDL_SCANCODE_RIGHT)) val += 1;
            return val;
        }
        if (axis == "vertical") {
            float val = 0;
            if (isKeyDown(SDL_SCANCODE_W) || isKeyDown(SDL_SCANCODE_UP)) val -= 1;
            if (isKeyDown(SDL_SCANCODE_S) || isKeyDown(SDL_SCANCODE_DOWN)) val += 1;
            return val;
        }
        return 0;
    }

private:
    Input() = default;
    std::unordered_map<int, bool> currentKeys_;
    std::unordered_map<int, bool> prevKeys_;
    std::unordered_map<uint8_t, bool> currentMouse_;
    std::unordered_map<uint8_t, bool> prevMouse_;
    Vec2 mousePos_;
    int mouseWheel_ = 0;
};

} // namespace Engine
