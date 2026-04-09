// Tests for AABB and SAT collision detection
#include "engine/Collision.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace Engine;

static bool approx(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ---- AABB tests ----

static void test_aabb_overlap() {
    Vec2 normal; float depth;

    // Overlapping boxes
    bool hit = CollisionSystem::testAABB(
        {0, 0}, {10, 10},   // box A
        {5, 5}, {10, 10},   // box B
        normal, depth
    );
    assert(hit);
    assert(depth == 5.0f); // overlap is 5 on both axes, equal so picks X or Y
}

static void test_aabb_no_overlap() {
    Vec2 normal; float depth;

    bool hit = CollisionSystem::testAABB(
        {0, 0}, {10, 10},
        {20, 20}, {10, 10},
        normal, depth
    );
    assert(!hit);
}

static void test_aabb_touching_edge() {
    Vec2 normal; float depth;

    // Exactly touching = no overlap (overlap would be 0)
    bool hit = CollisionSystem::testAABB(
        {0, 0}, {10, 10},
        {10, 0}, {10, 10},
        normal, depth
    );
    assert(!hit);
}

static void test_aabb_contained() {
    Vec2 normal; float depth;

    // Small box inside big box
    bool hit = CollisionSystem::testAABB(
        {0, 0}, {100, 100},
        {40, 40}, {20, 20},
        normal, depth
    );
    assert(hit);
}

static void test_aabb_normal_direction() {
    Vec2 normal; float depth;

    // Box B slightly to the right of A — should resolve horizontally
    CollisionSystem::testAABB(
        {0, 0}, {10, 10},
        {8, 0}, {10, 10},
        normal, depth
    );
    assert(approx(depth, 2.0f));
    assert(normal.x != 0); // horizontal resolution

    // Box B slightly below A — should resolve vertically
    CollisionSystem::testAABB(
        {0, 0}, {10, 10},
        {0, 8}, {10, 10},
        normal, depth
    );
    assert(approx(depth, 2.0f));
    assert(normal.y != 0); // vertical resolution
}

// ---- SAT tests ----

static void test_sat_squares_overlap() {
    // Two overlapping axis-aligned squares
    auto polyA = CollisionSystem::aabbToVertices({0, 0}, {10, 10});
    auto polyB = CollisionSystem::aabbToVertices({5, 5}, {10, 10});

    Vec2 normal; float depth;
    bool hit = CollisionSystem::testSAT(polyA, polyB, normal, depth);
    assert(hit);
    assert(approx(depth, 5.0f));
}

static void test_sat_squares_no_overlap() {
    auto polyA = CollisionSystem::aabbToVertices({0, 0}, {10, 10});
    auto polyB = CollisionSystem::aabbToVertices({20, 20}, {10, 10});

    Vec2 normal; float depth;
    bool hit = CollisionSystem::testSAT(polyA, polyB, normal, depth);
    assert(!hit);
}

static void test_sat_triangle_square() {
    // Triangle
    std::vector<Vec2> tri = {{5, 0}, {10, 10}, {0, 10}};
    // Square overlapping with it
    auto sq = CollisionSystem::aabbToVertices({3, 5}, {10, 10});

    Vec2 normal; float depth;
    bool hit = CollisionSystem::testSAT(tri, sq, normal, depth);
    assert(hit);
}

static void test_sat_triangle_no_overlap() {
    std::vector<Vec2> tri = {{5, 0}, {10, 10}, {0, 10}};
    auto sq = CollisionSystem::aabbToVertices({20, 20}, {5, 5});

    Vec2 normal; float depth;
    bool hit = CollisionSystem::testSAT(tri, sq, normal, depth);
    assert(!hit);
}

static void test_sat_rotated_polygon() {
    // Diamond (rotated square)
    std::vector<Vec2> diamond = {{5, 0}, {10, 5}, {5, 10}, {0, 5}};
    auto sq = CollisionSystem::aabbToVertices({3, 3}, {4, 4});

    Vec2 normal; float depth;
    bool hit = CollisionSystem::testSAT(diamond, sq, normal, depth);
    assert(hit);
}

static void test_world_vertices() {
    PolygonCollider poly;
    poly.vertices = {{-5, -5}, {5, -5}, {5, 5}, {-5, 5}};

    Transform t;
    t.position = {100, 100};
    t.scale = {1, 1};
    t.rotation = 0;

    auto verts = CollisionSystem::getWorldVertices(poly, t);
    assert(verts.size() == 4);
    assert(approx(verts[0].x, 95) && approx(verts[0].y, 95));
    assert(approx(verts[2].x, 105) && approx(verts[2].y, 105));
}

static void test_world_vertices_rotated() {
    PolygonCollider poly;
    poly.vertices = {{10, 0}, {0, 10}, {-10, 0}, {0, -10}};

    Transform t;
    t.position = {0, 0};
    t.scale = {1, 1};
    t.rotation = 90; // 90 degrees

    auto verts = CollisionSystem::getWorldVertices(poly, t);
    // After 90-degree rotation: (10,0) -> (0,10), (0,10) -> (-10,0), etc.
    assert(approx(verts[0].x, 0, 0.1f) && approx(verts[0].y, 10, 0.1f));
}

static void test_aabb_to_vertices() {
    auto verts = CollisionSystem::aabbToVertices({10, 20}, {30, 40});
    assert(verts.size() == 4);
    assert(verts[0].x == 10 && verts[0].y == 20);
    assert(verts[1].x == 40 && verts[1].y == 20);
    assert(verts[2].x == 40 && verts[2].y == 60);
    assert(verts[3].x == 10 && verts[3].y == 60);
}

// ---- ECS integration test ----

static void test_collision_system_with_ecs() {
    World world;

    Entity a = world.createEntity();
    world.addComponent<Transform>(a, {{0, 0}});
    world.addComponent<BoxCollider>(a, {{}, {10, 10}, false, true, "wall"});

    Entity b = world.createEntity();
    world.addComponent<Transform>(b, {{5, 5}});
    world.addComponent<BoxCollider>(b, {{}, {10, 10}, false, false, "player"});
    world.addComponent<Velocity>(b);

    bool callbackFired = false;
    CollisionSystem cs;
    cs.setCallback([&](const CollisionInfo& info) {
        callbackFired = true;
        assert(info.depth > 0);
    });

    cs.update(world);
    assert(callbackFired);

    // Player should have been pushed out (wall is static)
    auto& tB = world.getComponent<Transform>(b);
    // After resolution, should no longer overlap
    Vec2 posA = world.getComponent<Transform>(a).position;
    Vec2 posB = tB.position;
    auto& cA = world.getComponent<BoxCollider>(a);
    auto& cB = world.getComponent<BoxCollider>(b);

    bool stillOverlapping =
        posA.x < posB.x + cB.size.x && posA.x + cA.size.x > posB.x &&
        posA.y < posB.y + cB.size.y && posA.y + cA.size.y > posB.y;
    assert(!stillOverlapping);
}

int main() {
    test_aabb_overlap();
    test_aabb_no_overlap();
    test_aabb_touching_edge();
    test_aabb_contained();
    test_aabb_normal_direction();
    test_sat_squares_overlap();
    test_sat_squares_no_overlap();
    test_sat_triangle_square();
    test_sat_triangle_no_overlap();
    test_sat_rotated_polygon();
    test_world_vertices();
    test_world_vertices_rotated();
    test_aabb_to_vertices();
    test_collision_system_with_ecs();
    std::cout << "[PASS] test_collision: all tests passed" << std::endl;
    return 0;
}
