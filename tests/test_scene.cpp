// Tests for Scene management (headless)
#include "engine/Scene.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace Engine;

static std::string eventLog;

class TestScene : public Scene {
public:
    TestScene(const std::string& name) : name_(name) {}

    void onEnter() override { eventLog += name_ + ":enter "; }
    void onExit() override  { eventLog += name_ + ":exit "; }
    void update(float dt) override { eventLog += name_ + ":update "; }
    void render(Renderer& r) override { eventLog += name_ + ":render "; }

private:
    std::string name_;
};

static void test_add_and_switch() {
    eventLog.clear();
    SceneManager sm;

    auto sceneA = std::make_shared<TestScene>("A");
    auto sceneB = std::make_shared<TestScene>("B");
    sm.addScene("a", sceneA);
    sm.addScene("b", sceneB);

    sm.switchTo("a");
    // Switch is deferred — not applied until update
    assert(sm.getCurrentScene() == nullptr);

    sm.update(0.016f);
    assert(sm.getCurrentScene() == sceneA.get());
    assert(sm.getCurrentName() == "a");
    // Should have: enter + update
    assert(eventLog.find("A:enter") != std::string::npos);
    assert(eventLog.find("A:update") != std::string::npos);
}

static void test_scene_transition() {
    eventLog.clear();
    SceneManager sm;

    auto sceneA = std::make_shared<TestScene>("A");
    auto sceneB = std::make_shared<TestScene>("B");
    sm.addScene("a", sceneA);
    sm.addScene("b", sceneB);

    sm.switchTo("a");
    sm.update(0.016f);
    eventLog.clear();

    sm.switchTo("b");
    sm.update(0.016f);

    // A should exit, B should enter and update
    assert(eventLog.find("A:exit") != std::string::npos);
    assert(eventLog.find("B:enter") != std::string::npos);
    assert(eventLog.find("B:update") != std::string::npos);
    assert(sm.getCurrentScene() == sceneB.get());
}

static void test_render() {
    eventLog.clear();
    SceneManager sm;

    auto scene = std::make_shared<TestScene>("X");
    sm.addScene("x", scene);
    sm.switchTo("x");
    sm.update(0.016f);
    eventLog.clear();

    Renderer dummyRenderer; // not initialized — render just logs, doesn't draw
    sm.render(dummyRenderer);
    assert(eventLog.find("X:render") != std::string::npos);
}

static void test_switch_to_nonexistent() {
    SceneManager sm;
    auto scene = std::make_shared<TestScene>("A");
    sm.addScene("a", scene);
    sm.switchTo("a");
    sm.update(0.016f);

    // Switch to nonexistent — should not crash, keep current
    sm.switchTo("doesnt_exist");
    sm.update(0.016f);
    assert(sm.getCurrentName() == "a");
}

static void test_multiple_updates() {
    eventLog.clear();
    SceneManager sm;
    auto scene = std::make_shared<TestScene>("M");
    sm.addScene("m", scene);
    sm.switchTo("m");

    for (int i = 0; i < 5; i++) {
        sm.update(0.016f);
    }

    // Enter should happen once, update should happen 5 times
    size_t enterCount = 0, updateCount = 0;
    size_t pos = 0;
    while ((pos = eventLog.find("M:enter", pos)) != std::string::npos) { enterCount++; pos++; }
    pos = 0;
    while ((pos = eventLog.find("M:update", pos)) != std::string::npos) { updateCount++; pos++; }
    assert(enterCount == 1);
    assert(updateCount == 5);
}

int main() {
    test_add_and_switch();
    test_scene_transition();
    test_render();
    test_switch_to_nonexistent();
    test_multiple_updates();
    std::cout << "[PASS] test_scene: all tests passed" << std::endl;
    return 0;
}
