// Tests for Vec2, Rect, Color
#include "engine/Math.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace Engine;

static bool approx(float a, float b, float eps = 0.001f) {
    return std::fabs(a - b) < eps;
}

static void test_vec2_basics() {
    Vec2 a{3, 4};
    assert(approx(a.length(), 5.0f));
    assert(approx(a.lengthSq(), 25.0f));

    Vec2 b = a.normalized();
    assert(approx(b.length(), 1.0f));
    assert(approx(b.x, 0.6f));
    assert(approx(b.y, 0.8f));

    Vec2 zero;
    assert(zero.x == 0 && zero.y == 0);
    Vec2 zn = zero.normalized();
    assert(zn.x == 0 && zn.y == 0); // no division by zero
}

static void test_vec2_arithmetic() {
    Vec2 a{2, 3}, b{4, 1};
    Vec2 c = a + b; assert(c.x == 6 && c.y == 4);
    Vec2 d = a - b; assert(d.x == -2 && d.y == 2);
    Vec2 e = a * 3;  assert(e.x == 6 && e.y == 9);
    Vec2 f = a / 2;  assert(approx(f.x, 1.0f) && approx(f.y, 1.5f));

    float dot = a.dot(b);   assert(approx(dot, 11.0f));  // 2*4 + 3*1
    float cross = a.cross(b); assert(approx(cross, -10.0f)); // 2*1 - 3*4
}

static void test_vec2_lerp() {
    Vec2 a{0, 0}, b{10, 20};
    Vec2 mid = Vec2::lerp(a, b, 0.5f);
    assert(approx(mid.x, 5) && approx(mid.y, 10));

    Vec2 start = Vec2::lerp(a, b, 0.0f);
    assert(start.x == 0 && start.y == 0);
}

static void test_vec2_perp() {
    Vec2 a{1, 0};
    Vec2 p = a.perp();
    assert(p.x == 0 && p.y == 1);
    assert(approx(a.dot(p), 0)); // perpendicular => dot = 0
}

static void test_rect() {
    Rect r{10, 20, 100, 50};
    assert(r.contains({50, 40}));
    assert(!r.contains({5, 40}));
    assert(!r.contains({50, 80}));

    Vec2 c = r.center();
    assert(approx(c.x, 60) && approx(c.y, 45));

    Rect r2{50, 30, 80, 60};
    assert(r.intersects(r2));

    Rect r3{200, 200, 10, 10};
    assert(!r.intersects(r3));
}

static void test_color_lerp() {
    Color a{0, 0, 0, 255};
    Color b{200, 100, 50, 0};
    Color mid = Color::lerp(a, b, 0.5f);
    assert(mid.r == 100 && mid.g == 50 && mid.b == 25);

    // Clamping
    Color clamped = Color::lerp(a, b, 2.0f);
    assert(clamped.r == b.r && clamped.g == b.g);
}

int main() {
    test_vec2_basics();
    test_vec2_arithmetic();
    test_vec2_lerp();
    test_vec2_perp();
    test_rect();
    test_color_lerp();
    std::cout << "[PASS] test_math: all tests passed" << std::endl;
    return 0;
}
