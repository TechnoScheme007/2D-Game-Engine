#pragma once
#include "ECS.h"
#include "Renderer.h"
#include "Collision.h"
#include "ParticleSystem.h"
#include "TileMap.h"
#include <string>
#include <memory>

namespace Engine {

class Engine; // forward declaration

class Scene {
public:
    virtual ~Scene() = default;

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void update(float dt) = 0;
    virtual void render(Renderer& renderer) = 0;
    virtual void handleEvent(const SDL_Event& event) {}

    void setEngine(Engine* engine) { engine_ = engine; }

protected:
    Engine* engine_ = nullptr;
    World world_;
    ParticlePool particles_{1000};
    CollisionSystem collisionSystem_;
};

class SceneManager {
public:
    void addScene(const std::string& name, std::shared_ptr<Scene> scene);
    void switchTo(const std::string& name);
    void update(float dt);
    void render(Renderer& renderer);
    void handleEvent(const SDL_Event& event);

    Scene* getCurrentScene() const { return currentScene_.get(); }
    const std::string& getCurrentName() const { return currentName_; }

private:
    std::unordered_map<std::string, std::shared_ptr<Scene>> scenes_;
    std::shared_ptr<Scene> currentScene_;
    std::string currentName_;
    std::string pendingSwitch_;
};

} // namespace Engine
