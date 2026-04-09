# Blitz2D Engine

A lightweight, feature-complete 2D game engine written in **C++17** with **SDL2**. Designed around an Entity-Component-System architecture with a focus on simplicity, performance, and zero external asset dependencies.

Ships with **Dungeon Blaster**, a fully playable top-down shooter built entirely on the engine.

---

## Features

### Core Engine

- **Entity-Component-System** — sparse-set based ECS with type-safe component pools, compile-time queries via `view<T...>()` and `each<T...>(lambda)`, and automatic entity recycling
- **Fixed-Timestep Game Loop** — accumulator-based update with configurable tick rate, frame time capping to prevent spiral-of-death, and decoupled rendering
- **Scene Management** — scene stack with `onEnter`/`onExit` lifecycle hooks and deferred switching to prevent mid-frame state corruption

### Rendering

- **Sprite Renderer** — texture loading (BMP), procedural texture generation, color tinting, alpha blending, rotation, and flip support
- **Tile Map System** — grid-based tile maps with frustum-culled rendering, per-tile collision flags, and sweep-based physics resolution
- **Built-in Bitmap Font** — 5x7 pixel font with scalable rendering; no external font files required
- **Drawing Primitives** — filled/outlined rectangles, lines, points, circles
- **Camera** — smooth-follow with lerp, screen-to-world coordinate mapping, boundary clamping

### Physics & Input

- **AABB Collision Detection** — axis-aligned bounding box overlap with minimum-penetration-depth resolution and velocity correction
- **SAT Collision Detection** — Separating Axis Theorem for arbitrary convex polygons with rotation and scale support
- **Collision Callbacks** — trigger/solid distinction with per-collision event dispatching
- **Input System** — keyboard state tracking (pressed/held/released), mouse position and buttons, named axis abstraction (`"horizontal"`, `"vertical"`), scroll wheel

### Effects

- **Particle System** — object-pooled particles with start/end color interpolation, velocity damping, configurable emitter components that integrate directly with the ECS

---

## Demo Game: Dungeon Blaster

A top-down dungeon shooter demonstrating every engine system in action.

| | |
|---|---|
| **Movement** | `W` `A` `S` `D` |
| **Shoot** | `Space` or `Left Click` (last move direction) |
| **Aim + Shoot** | Arrow keys or `H` `J` `K` `L` |
| **Restart** | `R` (on game over) |

**Gameplay:** Navigate a dungeon, shoot enemies (+10 pts), collect coins (+25 pts). Clearing all coins advances to the next level with more enemies and more loot. You have 5 HP. All visuals are procedurally generated at runtime — no asset files needed.

---

## Building

### Prerequisites

| Tool | Version |
|------|---------|
| C++ compiler | C++17 support (GCC 8+, Clang 7+, MSVC 2017+) |
| CMake | 3.16+ |
| SDL2 | 2.0.12+ |

### Install SDL2

```bash
# Ubuntu / Debian
sudo apt install libsdl2-dev

# Arch Linux
sudo pacman -S sdl2

# macOS
brew install sdl2

# Windows (MSYS2)
pacman -S mingw-w64-ucrt-x86_64-SDL2

# Windows (vcpkg)
vcpkg install sdl2
```

### Build & Run

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./DungeonBlaster        # Linux / macOS
DungeonBlaster.exe      # Windows
```

### Run Tests

The engine ships with a headless test suite that validates every subsystem without opening a window:

```bash
cd build
ctest --output-on-failure
```

Or run individual tests directly:

```bash
./test_math             # Vec2, Rect, Color
./test_ecs              # Entity-Component-System
./test_collision         # AABB + SAT collision
./test_tilemap          # Tile map logic + collision
./test_particles        # Particle pool lifecycle
./test_scene            # Scene management transitions
```

All tests use plain `assert()` — zero external test framework dependencies.

> **Windows note:** Ensure the MinGW `bin/` directory is on your `PATH` so runtime DLLs (`libstdc++-6.dll`, etc.) are found.

---

## Project Structure

```
.
├── CMakeLists.txt
├── include/engine/          # Public engine headers
│   ├── Math.h               #   Vec2, Rect, Color
│   ├── ECS.h                #   Entity-Component-System
│   ├── Components.h         #   Built-in component types
│   ├── Engine.h             #   Core engine + game loop
│   ├── Renderer.h           #   SDL2 rendering + bitmap font
│   ├── Input.h              #   Keyboard & mouse input
│   ├── Collision.h          #   AABB + SAT collision
│   ├── TileMap.h            #   Tile map system
│   ├── ParticleSystem.h     #   Particle pool
│   └── Scene.h              #   Scene management
├── src/engine/              # Engine implementation
│   ├── Engine.cpp
│   ├── Renderer.cpp
│   ├── Collision.cpp
│   ├── TileMap.cpp
│   ├── ParticleSystem.cpp
│   └── Scene.cpp
├── src/demo/
│   └── DemoGame.cpp         # Demo game (Dungeon Blaster)
└── tests/                   # Headless engine tests
    ├── test_math.cpp        #   Vec2, Rect, Color
    ├── test_ecs.cpp         #   ECS: create, destroy, view, each
    ├── test_collision.cpp   #   AABB + SAT + ECS integration
    ├── test_tilemap.cpp     #   Tile ops, collision resolution
    ├── test_particles.cpp   #   Emit, expire, pool wrap, clear
    └── test_scene.cpp       #   Lifecycle, transitions, deferred switch
```

---

## Using the Engine

Create a scene, register it, and run:

```cpp
#include "engine/Engine.h"
#include "engine/ECS.h"
#include "engine/Components.h"
#include "engine/Scene.h"

class MyScene : public Engine::Scene {
public:
    void onEnter() override {
        auto player = world_.createEntity();
        world_.addComponent<Engine::Transform>(player, {{100, 200}});
        world_.addComponent<Engine::Velocity>(player);
        world_.addComponent<Engine::BoxCollider>(player, {{}, {32, 32}});
    }

    void update(float dt) override {
        auto& input = Engine::Input::instance();

        world_.each<Engine::Transform, Engine::Velocity>(
            [&](Engine::Entity e, Engine::Transform& t, Engine::Velocity& v) {
                t.position += v.velocity * dt;
            });

        collisionSystem_.update(world_);
        particles_.update(dt);
    }

    void render(Engine::Renderer& renderer) override {
        world_.each<Engine::Transform, Engine::BoxCollider>(
            [&](Engine::Entity e, Engine::Transform& t, Engine::BoxCollider& c) {
                renderer.drawRect({t.position.x, t.position.y, c.size.x, c.size.y},
                                  {255, 255, 255, 255}, true);
            });
        particles_.render(renderer);
    }
};

int main(int argc, char* argv[]) {
    Engine::Engine engine;
    engine.init({.title = "My Game", .windowWidth = 800, .windowHeight = 600});

    auto scene = std::make_shared<MyScene>();
    scene->setEngine(&engine);
    engine.getSceneManager().addScene("main", scene);
    engine.getSceneManager().switchTo("main");

    engine.run();
    return 0;
}
```

---

## Technical Details

### Game Loop

```
while running:
    frameTime = min(elapsed, 250ms)       // cap to prevent spiral of death
    accumulator += frameTime
    while accumulator >= fixedTimestep:    // default: 1/60s
        update(fixedTimestep)             // deterministic simulation
        accumulator -= fixedTimestep
    render()                              // vsync'd
```

### ECS Memory Layout

Components are stored in dense arrays per type. Entity-to-index mapping uses hash maps, giving O(1) add/remove/lookup while keeping iteration cache-friendly. Entity IDs are recycled via a free list.

### Collision

- **AABB**: Tests overlap on both axes simultaneously, resolves along the axis of minimum penetration, and corrects velocity to prevent tunneling.
- **SAT**: Projects both polygons onto each edge normal, finds the axis of minimum overlap. Supports arbitrary convex polygons with rotation and non-uniform scale.

---

## License

MIT
