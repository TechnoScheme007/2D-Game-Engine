// Tests for TileMap (headless — no rendering)
#include "engine/TileMap.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace Engine;

static bool approx(float a, float b, float eps = 0.5f) {
    return std::fabs(a - b) < eps;
}

static void test_init_and_dimensions() {
    TileMap map;
    map.init(20, 15, 32, "tiles");
    assert(map.getMapWidth() == 20);
    assert(map.getMapHeight() == 15);
    assert(map.getTileSize() == 32);
    assert(map.getPixelWidth() == 640);
    assert(map.getPixelHeight() == 480);
}

static void test_set_get_tile() {
    TileMap map;
    map.init(10, 10, 32, "tiles");

    map.setTile(3, 4, 1, true, 32, 0);
    Tile t = map.getTile(3, 4);
    assert(t.id == 1);
    assert(t.solid == true);
    assert(t.srcX == 32);
    assert(t.srcY == 0);

    // Default tile
    Tile empty = map.getTile(0, 0);
    assert(empty.id == -1);
    assert(empty.solid == false);
}

static void test_out_of_bounds() {
    TileMap map;
    map.init(10, 10, 32, "tiles");

    // Out of bounds returns solid sentinel
    Tile oob = map.getTile(-1, 0);
    assert(oob.id == -1);
    assert(oob.solid == true);

    Tile oob2 = map.getTile(10, 10);
    assert(oob2.solid == true);

    // Setting out of bounds should be a no-op
    map.setTile(-1, -1, 5, true);
    map.setTile(100, 100, 5, true);
}

static void test_is_solid() {
    TileMap map;
    map.init(10, 10, 32, "tiles");
    map.setTile(3, 3, 1, true);
    map.setTile(4, 3, 0, false);

    // World coords in tile (3,3) -> solid
    assert(map.isSolid(3 * 32 + 10, 3 * 32 + 10));
    // World coords in tile (4,3) -> not solid
    assert(!map.isSolid(4 * 32 + 10, 3 * 32 + 10));
}

static void test_check_collision() {
    TileMap map;
    map.init(10, 10, 32, "tiles");

    // Create a wall at tile (5, 5)
    map.setTile(5, 5, 1, true);

    // AABB overlapping the wall tile
    Rect overlap = {5 * 32.0f - 5, 5 * 32.0f - 5, 20, 20};
    assert(map.checkCollision(overlap));

    // AABB far from the wall
    Rect far = {0, 0, 20, 20};
    assert(!map.checkCollision(far));
}

static void test_resolve_collision() {
    TileMap map;
    map.init(10, 10, 32, "tiles");

    // Create a solid wall at tile (5, 5)
    map.setTile(5, 5, 1, true);

    // Object moving right into the wall
    Vec2 pos = {5 * 32.0f - 20, 5 * 32.0f};
    Vec2 size = {16, 16};
    Vec2 vel = {10, 0};

    Vec2 resolved = map.resolveCollision(pos, size, vel);
    // Should either pass through (if no collision) or be stopped
    Rect test = {resolved.x, resolved.y, size.x, size.y};
    assert(!map.checkCollision(test)); // resolved position must be valid
}

static void test_load_from_array() {
    TileMap map;
    map.init(4, 3, 32, "tiles");

    std::vector<std::vector<int>> data = {
        {1, 1, 1, 1},
        {1, 0, 0, 1},
        {1, 1, 1, 1},
    };
    std::vector<bool> solidTiles = {false, true}; // tile 0 = floor, tile 1 = wall

    map.loadFromArray(data, solidTiles, 4);

    assert(map.getTile(0, 0).id == 1);
    assert(map.getTile(0, 0).solid == true);
    assert(map.getTile(1, 1).id == 0);
    assert(map.getTile(1, 1).solid == false);
}

static void test_corridor_walk() {
    // Build a corridor and walk through it
    TileMap map;
    map.init(10, 5, 32, "tiles");

    // Walls on top and bottom
    for (int x = 0; x < 10; x++) {
        map.setTile(x, 0, 1, true);
        map.setTile(x, 4, 1, true);
    }
    // Floor in the middle
    for (int y = 1; y <= 3; y++)
        for (int x = 0; x < 10; x++)
            map.setTile(x, y, 0, false);

    // Walk across the corridor
    Vec2 pos = {32, 2 * 32.0f};
    Vec2 size = {16, 16};
    for (int step = 0; step < 20; step++) {
        Vec2 vel = {5, 0};
        pos = map.resolveCollision(pos, size, vel);
        Rect test = {pos.x, pos.y, size.x, size.y};
        assert(!map.checkCollision(test));
    }
}

int main() {
    test_init_and_dimensions();
    test_set_get_tile();
    test_out_of_bounds();
    test_is_solid();
    test_check_collision();
    test_resolve_collision();
    test_load_from_array();
    test_corridor_walk();
    std::cout << "[PASS] test_tilemap: all tests passed" << std::endl;
    return 0;
}
