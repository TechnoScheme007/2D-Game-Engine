#include "engine/Engine.h"
#include <iostream>
#include <SDL2/SDL.h>

namespace Engine {

Engine::~Engine() {
    renderer_.shutdown();
    SDL_Quit();
}

bool Engine::init(const EngineConfig& config) {
    config_ = config;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!renderer_.init(config.title, config.windowWidth, config.windowHeight, config.fullscreen)) {
        return false;
    }

    running_ = true;
    return true;
}

void Engine::run() {
    Uint64 currentTime = SDL_GetPerformanceCounter();
    Uint64 frequency = SDL_GetPerformanceFrequency();
    float accumulator = 0.0f;
    float fpsTimer = 0.0f;
    int frameCount = 0;

    while (running_) {
        Uint64 newTime = SDL_GetPerformanceCounter();
        float frameTime = static_cast<float>(newTime - currentTime) / static_cast<float>(frequency);
        currentTime = newTime;

        // Cap frame time to avoid spiral of death
        if (frameTime > 0.25f) frameTime = 0.25f;

        dt_ = frameTime;

        // FPS counter
        fpsTimer += frameTime;
        frameCount++;
        if (fpsTimer >= 1.0f) {
            fps_ = static_cast<float>(frameCount) / fpsTimer;
            frameCount = 0;
            fpsTimer = 0.0f;
        }

        processEvents();

        // Fixed timestep update
        accumulator += frameTime;
        int steps = 0;
        while (accumulator >= config_.fixedTimestep && steps < config_.maxFrameSkip) {
            sceneManager_.update(config_.fixedTimestep);
            accumulator -= config_.fixedTimestep;
            steps++;
        }

        Input::instance().update();

        render();
    }
}

void Engine::quit() {
    running_ = false;
}

void Engine::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running_ = false;
        }
        Input::instance().handleEvent(event);
        sceneManager_.handleEvent(event);
    }
}

void Engine::update(float dt) {
    sceneManager_.update(dt);
}

void Engine::render() {
    renderer_.clear();
    sceneManager_.render(renderer_);
    renderer_.present();
}

} // namespace Engine
