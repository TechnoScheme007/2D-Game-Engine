#pragma once
#include "Renderer.h"
#include "Input.h"
#include "Scene.h"
#include <string>
#include <functional>

namespace Engine {

struct EngineConfig {
    std::string title = "2D Game Engine";
    int windowWidth = 960;
    int windowHeight = 640;
    bool fullscreen = false;
    float fixedTimestep = 1.0f / 60.0f;
    int maxFrameSkip = 5;
};

class Engine {
public:
    Engine() = default;
    ~Engine();

    bool init(const EngineConfig& config = {});
    void run();
    void quit();

    Renderer& getRenderer() { return renderer_; }
    SceneManager& getSceneManager() { return sceneManager_; }
    Input& getInput() { return Input::instance(); }

    float getDeltaTime() const { return dt_; }
    float getFPS() const { return fps_; }
    bool isRunning() const { return running_; }

private:
    void processEvents();
    void update(float dt);
    void render();

    Renderer renderer_;
    SceneManager sceneManager_;
    EngineConfig config_;
    bool running_ = false;
    float dt_ = 0.0f;
    float fps_ = 0.0f;
};

} // namespace Engine
