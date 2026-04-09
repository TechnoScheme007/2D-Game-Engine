// Tests for ParticlePool (headless — logic only, no rendering)
#include "engine/ParticleSystem.h"
#include <cassert>
#include <iostream>

using namespace Engine;

static void test_emit_and_count() {
    ParticlePool pool(100);
    assert(pool.activeCount() == 0);

    pool.emit({0, 0}, {10, 0}, 4, {255, 255, 255, 255}, {0, 0, 0, 0}, 1.0f);
    assert(pool.activeCount() == 1);

    for (int i = 0; i < 9; i++) {
        pool.emit({0, 0}, {10, 0}, 4, {255, 0, 0, 255}, {0, 0, 0, 0}, 1.0f);
    }
    assert(pool.activeCount() == 10);
}

static void test_particles_expire() {
    ParticlePool pool(100);

    pool.emit({0, 0}, {10, 0}, 4, {255, 255, 255, 255}, {0, 0, 0, 0}, 0.5f);
    assert(pool.activeCount() == 1);

    // Advance past lifetime
    pool.update(0.6f);
    assert(pool.activeCount() == 0);
}

static void test_particles_move() {
    ParticlePool pool(100);
    pool.emit({0, 0}, {100, 0}, 4, {255, 255, 255, 255}, {0, 0, 0, 0}, 2.0f);

    // After update, particle should have moved
    pool.update(0.1f);
    assert(pool.activeCount() == 1);
    // Can't easily inspect position from outside, but at least it didn't crash
}

static void test_pool_wraps() {
    ParticlePool pool(5);

    // Emit more than capacity — should wrap, not crash
    for (int i = 0; i < 20; i++) {
        pool.emit({0, 0}, {1, 0}, 2, {255, 0, 0, 255}, {0, 0, 0, 0}, 10.0f);
    }
    assert(pool.activeCount() == 5); // capped at pool size
}

static void test_clear() {
    ParticlePool pool(100);
    for (int i = 0; i < 50; i++) {
        pool.emit({0, 0}, {1, 0}, 2, {255, 0, 0, 255}, {0, 0, 0, 0}, 10.0f);
    }
    assert(pool.activeCount() == 50);

    pool.clear();
    assert(pool.activeCount() == 0);
}

static void test_mixed_lifetimes() {
    ParticlePool pool(100);

    pool.emit({0, 0}, {0, 0}, 2, {255, 0, 0, 255}, {0, 0, 0, 0}, 0.2f); // short
    pool.emit({0, 0}, {0, 0}, 2, {0, 255, 0, 255}, {0, 0, 0, 0}, 1.0f); // long
    pool.emit({0, 0}, {0, 0}, 2, {0, 0, 255, 255}, {0, 0, 0, 0}, 0.5f); // medium
    assert(pool.activeCount() == 3);

    pool.update(0.3f);
    assert(pool.activeCount() == 2); // short one expired

    pool.update(0.3f);
    assert(pool.activeCount() == 1); // medium one expired

    pool.update(0.5f);
    assert(pool.activeCount() == 0); // all expired
}

int main() {
    test_emit_and_count();
    test_particles_expire();
    test_particles_move();
    test_pool_wraps();
    test_clear();
    test_mixed_lifetimes();
    std::cout << "[PASS] test_particles: all tests passed" << std::endl;
    return 0;
}
