#include "engine/Scene.h"

namespace Engine {

void SceneManager::addScene(const std::string& name, std::shared_ptr<Scene> scene) {
    scenes_[name] = scene;
}

void SceneManager::switchTo(const std::string& name) {
    pendingSwitch_ = name;
}

void SceneManager::update(float dt) {
    if (!pendingSwitch_.empty()) {
        auto it = scenes_.find(pendingSwitch_);
        if (it != scenes_.end()) {
            if (currentScene_) currentScene_->onExit();
            currentScene_ = it->second;
            currentName_ = pendingSwitch_;
            currentScene_->onEnter();
        }
        pendingSwitch_.clear();
    }

    if (currentScene_) currentScene_->update(dt);
}

void SceneManager::render(Renderer& renderer) {
    if (currentScene_) currentScene_->render(renderer);
}

void SceneManager::handleEvent(const SDL_Event& event) {
    if (currentScene_) currentScene_->handleEvent(event);
}

} // namespace Engine
