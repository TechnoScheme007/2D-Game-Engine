// Tests for Entity-Component-System
#include "engine/ECS.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace Engine;

struct Position { float x, y; };
struct Name { std::string value; };
struct Health { int hp; };

static void test_create_destroy() {
    World w;
    Entity e1 = w.createEntity();
    Entity e2 = w.createEntity();
    assert(e1 != e2);
    assert(e1 != NULL_ENTITY);
    assert(w.isAlive(e1));
    assert(w.isAlive(e2));

    w.destroyEntity(e1);
    assert(!w.isAlive(e1));
    assert(w.isAlive(e2));
}

static void test_entity_recycling() {
    World w;
    Entity e1 = w.createEntity();
    w.destroyEntity(e1);

    Entity e2 = w.createEntity();
    assert(e2 == e1); // recycled ID
    assert(w.isAlive(e2));
}

static void test_add_get_component() {
    World w;
    Entity e = w.createEntity();

    w.addComponent<Position>(e, {10.0f, 20.0f});
    assert(w.hasComponent<Position>(e));
    assert(!w.hasComponent<Name>(e));

    auto& pos = w.getComponent<Position>(e);
    assert(pos.x == 10.0f && pos.y == 20.0f);

    pos.x = 99.0f;
    assert(w.getComponent<Position>(e).x == 99.0f); // reference works
}

static void test_remove_component() {
    World w;
    Entity e = w.createEntity();
    w.addComponent<Position>(e, {1, 2});
    w.addComponent<Name>(e, {"test"});

    assert(w.hasComponent<Position>(e));
    assert(w.hasComponent<Name>(e));

    w.removeComponent<Position>(e);
    assert(!w.hasComponent<Position>(e));
    assert(w.hasComponent<Name>(e)); // other component untouched
}

static void test_view() {
    World w;
    Entity e1 = w.createEntity();
    w.addComponent<Position>(e1, {0, 0});
    w.addComponent<Name>(e1, {"player"});

    Entity e2 = w.createEntity();
    w.addComponent<Position>(e2, {5, 5});

    Entity e3 = w.createEntity();
    w.addComponent<Name>(e3, {"label"});

    // View with Position only -> e1, e2
    auto posEntities = w.view<Position>();
    assert(posEntities.size() == 2);

    // View with Position AND Name -> e1 only
    auto both = w.view<Position, Name>();
    assert(both.size() == 1);
    assert(both[0] == e1);

    // View with Name only -> e1, e3
    auto nameEntities = w.view<Name>();
    assert(nameEntities.size() == 2);
}

static void test_each() {
    World w;
    for (int i = 0; i < 5; i++) {
        Entity e = w.createEntity();
        w.addComponent<Position>(e, {static_cast<float>(i), 0});
        w.addComponent<Health>(e, {100});
    }
    // Add one entity with Position only (no Health)
    Entity extra = w.createEntity();
    w.addComponent<Position>(extra, {99, 99});

    int count = 0;
    float sumX = 0;
    w.each<Position, Health>([&](Entity e, Position& p, Health& h) {
        count++;
        sumX += p.x;
        h.hp -= 10;
    });

    assert(count == 5);
    assert(sumX == 0 + 1 + 2 + 3 + 4);

    // Verify mutation worked
    w.each<Health>([&](Entity e, Health& h) {
        assert(h.hp == 90);
    });
}

static void test_destroy_with_components() {
    World w;
    Entity e = w.createEntity();
    w.addComponent<Position>(e, {1, 2});
    w.addComponent<Name>(e, {"test"});

    w.destroyEntity(e);
    assert(!w.isAlive(e));

    // Creating a new entity should work fine
    Entity e2 = w.createEntity();
    w.addComponent<Position>(e2, {3, 4});
    assert(w.getComponent<Position>(e2).x == 3);
}

static void test_many_entities() {
    World w;
    for (int i = 0; i < 1000; i++) {
        Entity e = w.createEntity();
        w.addComponent<Position>(e, {static_cast<float>(i), 0});
    }

    auto all = w.view<Position>();
    assert(all.size() == 1000);

    // Destroy half
    for (int i = 0; i < 500; i++) {
        w.destroyEntity(all[i]);
    }

    auto remaining = w.view<Position>();
    assert(remaining.size() == 500);
}

static void test_clear() {
    World w;
    for (int i = 0; i < 10; i++) {
        Entity e = w.createEntity();
        w.addComponent<Position>(e, {0, 0});
    }
    w.clear();
    assert(w.getAllEntities().empty());
    assert(w.view<Position>().empty());

    // Should work fine after clear
    Entity e = w.createEntity();
    w.addComponent<Position>(e, {1, 1});
    assert(w.view<Position>().size() == 1);
}

int main() {
    test_create_destroy();
    test_entity_recycling();
    test_add_get_component();
    test_remove_component();
    test_view();
    test_each();
    test_destroy_with_components();
    test_many_entities();
    test_clear();
    std::cout << "[PASS] test_ecs: all tests passed" << std::endl;
    return 0;
}
