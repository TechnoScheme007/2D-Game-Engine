#include "engine/Engine.h"
#include "engine/ECS.h"
#include "engine/Components.h"
#include "engine/Collision.h"
#include "engine/TileMap.h"
#include "engine/ParticleSystem.h"
#include "engine/Scene.h"
#include "engine/Input.h"
#include "engine/Math.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace Engine;

// =====================================================================
// Constants
// =====================================================================
static constexpr int TILE_SIZE = 32;
static constexpr int MAP_W = 30;
static constexpr int MAP_H = 20;
static constexpr float PLAYER_SPEED = 180.0f;
static constexpr float ENEMY_SPEED = 60.0f;
static constexpr float BULLET_SPEED = 350.0f;
static constexpr int SCREEN_W = 960;
static constexpr int SCREEN_H = 640;

// =====================================================================
// Helper: random float [lo, hi]
// =====================================================================
static float randFloat(float lo, float hi) {
    return lo + static_cast<float>(rand()) / RAND_MAX * (hi - lo);
}

// =====================================================================
// Procedural texture generation (no external assets needed)
// =====================================================================
static void createGameTextures(Renderer& r) {
    // Player - blue square with white center
    {
        SDL_Surface* s = SDL_CreateRGBSurface(0, 24, 24, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        SDL_FillRect(s, nullptr, SDL_MapRGBA(s->format, 50, 100, 220, 255));
        SDL_Rect inner = {4, 4, 16, 16};
        SDL_FillRect(s, &inner, SDL_MapRGBA(s->format, 150, 200, 255, 255));
        SDL_Rect eye1 = {7, 8, 3, 3};
        SDL_Rect eye2 = {14, 8, 3, 3};
        SDL_FillRect(s, &eye1, SDL_MapRGBA(s->format, 255, 255, 255, 255));
        SDL_FillRect(s, &eye2, SDL_MapRGBA(s->format, 255, 255, 255, 255));
        SDL_Texture* t = SDL_CreateTextureFromSurface(r.getSDLRenderer(), s);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(s);
        // Manually register
        // We'll use createSolidTexture as a workaround, but actually let's just use the raw SDL
        // Instead, let's draw procedurally using the renderer's primitive drawing
    }
    // Actually, let's use solid color textures and draw everything with primitives for simplicity
    r.createSolidTexture("white", 4, 4, {255, 255, 255, 255});
    r.createSolidTexture("bullet", 4, 4, {255, 255, 100, 255});
}

// =====================================================================
// Custom components for the demo
// =====================================================================
struct Enemy {
    float changeTimer = 0;
    Vec2 dir;
    int scoreValue = 10;
};

struct Bullet {
    Vec2 direction;
    float lifetime = 2.0f;
};

struct Coin {
    float bobTimer = 0;
};

struct PlayerTag {};

// =====================================================================
// Tile map generation
// =====================================================================
static void generateMap(TileMap& map, Renderer& renderer) {
    // Create tileset texture: a strip of colored tiles
    // Tile 0 = floor (dark gray), Tile 1 = wall (brown), Tile 2 = floor variant
    {
        int tw = TILE_SIZE * 4;
        int th = TILE_SIZE;
        SDL_Surface* s = SDL_CreateRGBSurface(0, tw, th, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

        // Tile 0: dark floor
        SDL_Rect r0 = {0, 0, TILE_SIZE, TILE_SIZE};
        SDL_FillRect(s, &r0, SDL_MapRGBA(s->format, 40, 40, 50, 255));
        // Add subtle detail
        for (int i = 0; i < 8; i++) {
            SDL_Rect dot = {rand() % TILE_SIZE, rand() % TILE_SIZE, 2, 2};
            SDL_FillRect(s, &dot, SDL_MapRGBA(s->format, 50, 50, 60, 255));
        }

        // Tile 1: wall
        SDL_Rect r1 = {TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
        SDL_FillRect(s, &r1, SDL_MapRGBA(s->format, 80, 60, 40, 255));
        // wall detail - border
        SDL_Rect wb = {TILE_SIZE + 1, 1, TILE_SIZE - 2, TILE_SIZE - 2};
        SDL_FillRect(s, &wb, SDL_MapRGBA(s->format, 100, 75, 50, 255));
        SDL_Rect wc = {TILE_SIZE + 3, 3, TILE_SIZE - 6, TILE_SIZE - 6};
        SDL_FillRect(s, &wc, SDL_MapRGBA(s->format, 85, 65, 45, 255));

        // Tile 2: lighter floor
        SDL_Rect r2 = {TILE_SIZE * 2, 0, TILE_SIZE, TILE_SIZE};
        SDL_FillRect(s, &r2, SDL_MapRGBA(s->format, 50, 50, 60, 255));

        // Tile 3: accent floor
        SDL_Rect r3 = {TILE_SIZE * 3, 0, TILE_SIZE, TILE_SIZE};
        SDL_FillRect(s, &r3, SDL_MapRGBA(s->format, 45, 45, 55, 255));

        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer.getSDLRenderer(), s);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(s);

        // We need to register this texture with the renderer - use a small hack
        // Actually, let's create it properly through the renderer
    }

    // Create tileset via renderer's solid texture method isn't great for multi-tile.
    // Instead, let's render tiles as colored rectangles directly and skip texture-based tilemap rendering.
    // We'll override the tilemap render with our own.

    map.init(MAP_W, MAP_H, TILE_SIZE, "tileset");

    // Generate dungeon layout
    // Border walls
    for (int x = 0; x < MAP_W; x++) {
        map.setTile(x, 0, 1, true, TILE_SIZE, 0);
        map.setTile(x, MAP_H - 1, 1, true, TILE_SIZE, 0);
    }
    for (int y = 0; y < MAP_H; y++) {
        map.setTile(0, y, 1, true, TILE_SIZE, 0);
        map.setTile(MAP_W - 1, y, 1, true, TILE_SIZE, 0);
    }

    // Fill interior with floor
    for (int y = 1; y < MAP_H - 1; y++) {
        for (int x = 1; x < MAP_W - 1; x++) {
            int floorType = (x + y) % 3 == 0 ? 2 : 0;
            map.setTile(x, y, floorType, false, floorType * TILE_SIZE, 0);
        }
    }

    // Add some interior walls/obstacles
    // Horizontal walls
    for (int x = 4; x < 10; x++) map.setTile(x, 5, 1, true, TILE_SIZE, 0);
    for (int x = 15; x < 22; x++) map.setTile(x, 8, 1, true, TILE_SIZE, 0);
    for (int x = 5; x < 12; x++) map.setTile(x, 14, 1, true, TILE_SIZE, 0);

    // Vertical walls
    for (int y = 3; y < 8; y++) map.setTile(20, y, 1, true, TILE_SIZE, 0);
    for (int y = 10; y < 16; y++) map.setTile(12, y, 1, true, TILE_SIZE, 0);
    for (int y = 5; y < 10; y++) map.setTile(25, y, 1, true, TILE_SIZE, 0);

    // Some pillars
    map.setTile(7, 10, 1, true, TILE_SIZE, 0);
    map.setTile(17, 4, 1, true, TILE_SIZE, 0);
    map.setTile(23, 14, 1, true, TILE_SIZE, 0);
    map.setTile(5, 17, 1, true, TILE_SIZE, 0);
}

// =====================================================================
// Custom tilemap renderer (draw colored rects, no texture file needed)
// =====================================================================
static void renderTileMap(const TileMap& map, Renderer& renderer) {
    Vec2 cam = renderer.getCamera();
    int startX = std::max(0, static_cast<int>(cam.x) / TILE_SIZE);
    int startY = std::max(0, static_cast<int>(cam.y) / TILE_SIZE);
    int endX = std::min(map.getMapWidth(), static_cast<int>(cam.x + renderer.getWidth()) / TILE_SIZE + 2);
    int endY = std::min(map.getMapHeight(), static_cast<int>(cam.y + renderer.getHeight()) / TILE_SIZE + 2);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            Tile tile = map.getTile(x, y);
            if (tile.id < 0) continue;

            Rect r = {static_cast<float>(x * TILE_SIZE), static_cast<float>(y * TILE_SIZE),
                      static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE)};

            if (tile.solid) {
                // Wall
                renderer.drawRect(r, {100, 75, 50, 255}, true);
                Rect inner = {r.x + 2, r.y + 2, r.w - 4, r.h - 4};
                renderer.drawRect(inner, {85, 65, 45, 255}, true);
                Rect highlight = {r.x + 1, r.y + 1, r.w - 2, 2};
                renderer.drawRect(highlight, {120, 90, 60, 255}, true);
            } else {
                // Floor
                Color floorColor;
                if (tile.id == 0) floorColor = {40, 40, 50, 255};
                else if (tile.id == 2) floorColor = {50, 50, 60, 255};
                else floorColor = {45, 45, 55, 255};
                renderer.drawRect(r, floorColor, true);

                // Subtle grid lines
                renderer.drawLine({r.x, r.y}, {r.x + r.w, r.y}, {35, 35, 45, 255});
                renderer.drawLine({r.x, r.y}, {r.x, r.y + r.h}, {35, 35, 45, 255});
            }
        }
    }
}

// =====================================================================
// GameScene
// =====================================================================
class GameScene : public Scene {
public:
    void onEnter() override {
        srand(static_cast<unsigned>(time(nullptr)));
        score_ = 0;
        playerHealth_ = 5;
        gameOver_ = false;
        level_ = 1;
        shootCooldown_ = 0;
        enemySpawnTimer_ = 0;

        generateMap(tileMap_, engine_->getRenderer());
        createGameTextures(engine_->getRenderer());
        spawnPlayer();
        spawnCoins(8);
        spawnEnemies(4);

        collisionSystem_.setCallback([this](const CollisionInfo& info) {
            handleCollision(info);
        });
    }

    void onExit() override {
        world_.clear();
        particles_.clear();
    }

    void update(float dt) override {
        if (gameOver_) {
            if (Input::instance().isKeyPressed(SDL_SCANCODE_R)) {
                onExit();
                onEnter();
            }
            return;
        }

        updatePlayer(dt);
        updateEnemies(dt);
        updateBullets(dt);
        updateCoins(dt);
        updateParticleEmitters(dt);
        particles_.update(dt);

        // Physics: apply velocity
        world_.each<Transform, Velocity>([&](Entity e, Transform& t, Velocity& v) {
            Vec2 movement = v.velocity * dt;
            Vec2 size = {20.0f, 20.0f};
            if (world_.hasComponent<BoxCollider>(e)) {
                auto& col = world_.getComponent<BoxCollider>(e);
                size = col.size;
            }
            // Tile collision
            Vec2 newPos = tileMap_.resolveCollision(t.position + col_offset(e), size, movement);
            t.position = newPos - col_offset(e);

            // Apply drag
            v.velocity *= (1.0f - v.drag * dt);
        });

        // Entity-entity collision
        collisionSystem_.update(world_);

        // Camera follow player
        if (world_.isAlive(player_)) {
            auto& pt = world_.getComponent<Transform>(player_);
            Vec2 target = pt.position - Vec2{SCREEN_W / 2.0f, SCREEN_H / 2.0f};
            // Clamp camera to map bounds
            target.x = std::clamp(target.x, 0.0f,
                static_cast<float>(tileMap_.getPixelWidth() - SCREEN_W));
            target.y = std::clamp(target.y, 0.0f,
                static_cast<float>(tileMap_.getPixelHeight() - SCREEN_H));
            Vec2 cam = engine_->getRenderer().getCamera();
            cam = Vec2::lerp(cam, target, dt * 5.0f);
            engine_->getRenderer().setCamera(cam);
        }

        // Enemy spawning
        enemySpawnTimer_ += dt;
        if (enemySpawnTimer_ > 5.0f) {
            spawnEnemies(1);
            enemySpawnTimer_ = 0;
        }

        // Clean up dead entities
        cleanupEntities();

        // Check if all coins collected -> next level
        if (world_.view<Coin>().empty()) {
            level_++;
            spawnCoins(8 + level_ * 2);
            spawnEnemies(2 + level_);
        }
    }

    void render(Renderer& renderer) override {
        renderTileMap(tileMap_, renderer);
        renderCoins(renderer);
        renderEnemies(renderer);
        renderBullets(renderer);
        renderPlayer(renderer);
        particles_.render(renderer);
        renderHUD(renderer);
    }

private:
    TileMap tileMap_;
    Entity player_ = NULL_ENTITY;
    int score_ = 0;
    int playerHealth_ = 5;
    bool gameOver_ = false;
    int level_ = 1;
    float shootCooldown_ = 0;
    float enemySpawnTimer_ = 0;
    Vec2 lastShootDir_ = {1, 0};
    std::vector<Entity> toDestroy_;

    Vec2 col_offset(Entity e) {
        if (world_.hasComponent<BoxCollider>(e))
            return world_.getComponent<BoxCollider>(e).offset;
        return {};
    }

    // ----- Player -----
    void spawnPlayer() {
        player_ = world_.createEntity();
        auto& t = world_.addComponent<Transform>(player_);
        t.position = {3 * TILE_SIZE + 4.0f, 3 * TILE_SIZE + 4.0f};
        world_.addComponent<Velocity>(player_);

        auto& col = world_.addComponent<BoxCollider>(player_);
        col.size = {24, 24};
        col.tag = "player";

        world_.addComponent<PlayerTag>(player_);
    }

    void updatePlayer(float dt) {
        if (!world_.isAlive(player_)) return;
        auto& input = Input::instance();
        auto& vel = world_.getComponent<Velocity>(player_);

        Vec2 move;
        move.x = input.getAxis("horizontal");
        move.y = input.getAxis("vertical");

        if (move.lengthSq() > 0) {
            move = move.normalized() * PLAYER_SPEED;
            lastShootDir_ = move.normalized();
        }
        vel.velocity = move;

        // Shooting
        shootCooldown_ -= dt;
        if (shootCooldown_ <= 0) {
            Vec2 shootDir;
            bool shoot = false;

            if (input.isKeyDown(SDL_SCANCODE_UP) || input.isKeyDown(SDL_SCANCODE_K))
                { shootDir = {0, -1}; shoot = true; }
            if (input.isKeyDown(SDL_SCANCODE_DOWN) || input.isKeyDown(SDL_SCANCODE_J))
                { shootDir = {0, 1}; shoot = true; }
            if (input.isKeyDown(SDL_SCANCODE_LEFT) || input.isKeyDown(SDL_SCANCODE_H))
                { shootDir = {-1, 0}; shoot = true; }
            if (input.isKeyDown(SDL_SCANCODE_RIGHT) || input.isKeyDown(SDL_SCANCODE_L))
                { shootDir = {1, 0}; shoot = true; }

            if (input.isKeyDown(SDL_SCANCODE_SPACE) || input.isMouseDown(SDL_BUTTON_LEFT)) {
                shootDir = lastShootDir_;
                shoot = true;
            }

            if (shoot) {
                auto& pt = world_.getComponent<Transform>(player_);
                spawnBullet(pt.position + Vec2{12, 12}, shootDir);
                shootCooldown_ = 0.2f;
            }
        }
    }

    void renderPlayer(Renderer& renderer) {
        if (!world_.isAlive(player_)) return;
        auto& t = world_.getComponent<Transform>(player_);

        // Body
        Rect body = {t.position.x, t.position.y, 24, 24};
        renderer.drawRect(body, {50, 100, 220, 255}, true);
        // Inner
        Rect inner = {t.position.x + 4, t.position.y + 4, 16, 16};
        renderer.drawRect(inner, {100, 160, 255, 255}, true);
        // Eyes
        Rect eye1 = {t.position.x + 7, t.position.y + 8, 3, 3};
        Rect eye2 = {t.position.x + 14, t.position.y + 8, 3, 3};
        renderer.drawRect(eye1, {255, 255, 255, 255}, true);
        renderer.drawRect(eye2, {255, 255, 255, 255}, true);
        // Direction indicator
        Vec2 center = {t.position.x + 12, t.position.y + 12};
        Vec2 tip = center + lastShootDir_ * 14.0f;
        renderer.drawLine(center, tip, {255, 255, 100, 255});
    }

    // ----- Enemies -----
    void spawnEnemies(int count) {
        for (int i = 0; i < count; i++) {
            Entity e = world_.createEntity();
            auto& t = world_.addComponent<Transform>(e);
            // Find a non-solid spawn position away from player
            for (int attempt = 0; attempt < 50; attempt++) {
                float sx = randFloat(2, MAP_W - 2) * TILE_SIZE;
                float sy = randFloat(2, MAP_H - 2) * TILE_SIZE;
                Rect test = {sx, sy, 20, 20};
                if (!tileMap_.checkCollision(test)) {
                    if (world_.isAlive(player_)) {
                        auto& pt = world_.getComponent<Transform>(player_);
                        if ((Vec2{sx, sy} - pt.position).lengthSq() > 200 * 200) {
                            t.position = {sx, sy};
                            break;
                        }
                    } else {
                        t.position = {sx, sy};
                        break;
                    }
                }
            }

            world_.addComponent<Velocity>(e);
            auto& col = world_.addComponent<BoxCollider>(e);
            col.size = {20, 20};
            col.tag = "enemy";

            auto& enemy = world_.addComponent<Enemy>(e);
            enemy.changeTimer = randFloat(0.5f, 2.0f);
            enemy.dir = Vec2{randFloat(-1, 1), randFloat(-1, 1)}.normalized();

            auto& pe = world_.addComponent<ParticleEmitter>(e);
            pe.emitRate = 5;
            pe.lifetime = 0.5f;
            pe.speed = 20;
            pe.particleSize = 3;
            pe.startColor = {200, 50, 50, 200};
            pe.endColor = {100, 20, 20, 0};
            pe.spread = 360;
        }
    }

    void updateEnemies(float dt) {
        world_.each<Transform, Velocity, Enemy>([&](Entity e, Transform& t, Velocity& v, Enemy& enemy) {
            enemy.changeTimer -= dt;
            if (enemy.changeTimer <= 0) {
                // Change direction, sometimes chase player
                if (world_.isAlive(player_) && rand() % 3 == 0) {
                    auto& pt = world_.getComponent<Transform>(player_);
                    enemy.dir = (pt.position - t.position).normalized();
                } else {
                    enemy.dir = Vec2{randFloat(-1, 1), randFloat(-1, 1)}.normalized();
                }
                enemy.changeTimer = randFloat(1.0f, 3.0f);
            }
            v.velocity = enemy.dir * ENEMY_SPEED;
        });
    }

    void renderEnemies(Renderer& renderer) {
        world_.each<Transform, Enemy>([&](Entity e, Transform& t, Enemy& enemy) {
            // Red enemy body
            Rect body = {t.position.x, t.position.y, 20, 20};
            renderer.drawRect(body, {180, 40, 40, 255}, true);
            // Dark inner
            Rect inner = {t.position.x + 3, t.position.y + 3, 14, 14};
            renderer.drawRect(inner, {220, 60, 60, 255}, true);
            // Eyes
            Rect eye1 = {t.position.x + 5, t.position.y + 6, 3, 3};
            Rect eye2 = {t.position.x + 12, t.position.y + 6, 3, 3};
            renderer.drawRect(eye1, {255, 200, 200, 255}, true);
            renderer.drawRect(eye2, {255, 200, 200, 255}, true);
        });
    }

    // ----- Bullets -----
    void spawnBullet(const Vec2& pos, const Vec2& dir) {
        Entity e = world_.createEntity();
        auto& t = world_.addComponent<Transform>(e);
        t.position = pos;

        auto& v = world_.addComponent<Velocity>(e);
        v.velocity = dir.normalized() * BULLET_SPEED;

        auto& col = world_.addComponent<BoxCollider>(e);
        col.size = {6, 6};
        col.tag = "bullet";
        col.isTrigger = true;

        auto& b = world_.addComponent<Bullet>(e);
        b.direction = dir;

        // Muzzle flash particles
        for (int i = 0; i < 5; i++) {
            float angle = randFloat(0, 6.28f);
            Vec2 pv = {std::cos(angle) * randFloat(20, 60), std::sin(angle) * randFloat(20, 60)};
            particles_.emit(pos, pv, 2, {255, 255, 150, 255}, {255, 100, 0, 0}, randFloat(0.1f, 0.3f));
        }
    }

    void updateBullets(float dt) {
        world_.each<Transform, Bullet>([&](Entity e, Transform& t, Bullet& b) {
            b.lifetime -= dt;
            if (b.lifetime <= 0) {
                toDestroy_.push_back(e);
                return;
            }
            // Check tile collision
            Rect bulletRect = {t.position.x, t.position.y, 6, 6};
            if (tileMap_.checkCollision(bulletRect)) {
                // Wall hit particles
                for (int i = 0; i < 8; i++) {
                    float angle = randFloat(0, 6.28f);
                    Vec2 pv = {std::cos(angle) * randFloat(30, 80), std::sin(angle) * randFloat(30, 80)};
                    particles_.emit(t.position, pv, 3, {200, 180, 100, 255}, {100, 80, 40, 0}, randFloat(0.2f, 0.5f));
                }
                toDestroy_.push_back(e);
            }
        });
    }

    void renderBullets(Renderer& renderer) {
        world_.each<Transform, Bullet>([&](Entity e, Transform& t, Bullet& b) {
            Rect r = {t.position.x - 1, t.position.y - 1, 8, 8};
            renderer.drawRect(r, {255, 255, 100, 255}, true);
            // Glow
            Rect glow = {t.position.x - 3, t.position.y - 3, 12, 12};
            renderer.drawRect(glow, {255, 200, 50, 80}, true);
        });
    }

    // ----- Coins -----
    void spawnCoins(int count) {
        for (int i = 0; i < count; i++) {
            Entity e = world_.createEntity();
            auto& t = world_.addComponent<Transform>(e);
            for (int attempt = 0; attempt < 50; attempt++) {
                float sx = randFloat(2, MAP_W - 2) * TILE_SIZE;
                float sy = randFloat(2, MAP_H - 2) * TILE_SIZE;
                Rect test = {sx, sy, 12, 12};
                if (!tileMap_.checkCollision(test)) {
                    t.position = {sx, sy};
                    break;
                }
            }

            auto& col = world_.addComponent<BoxCollider>(e);
            col.size = {12, 12};
            col.offset = {4, 4};
            col.tag = "coin";
            col.isTrigger = true;

            auto& c = world_.addComponent<Coin>(e);
            c.bobTimer = randFloat(0, 6.28f);
        }
    }

    void updateCoins(float dt) {
        world_.each<Coin>([&](Entity e, Coin& c) {
            c.bobTimer += dt * 3.0f;
        });
    }

    void renderCoins(Renderer& renderer) {
        world_.each<Transform, Coin>([&](Entity e, Transform& t, Coin& c) {
            float bob = std::sin(c.bobTimer) * 3.0f;
            Vec2 pos = {t.position.x + 4, t.position.y + 4 + bob};

            // Coin body
            Rect body = {pos.x, pos.y, 12, 12};
            renderer.drawRect(body, {255, 200, 0, 255}, true);
            // Shine
            Rect shine = {pos.x + 2, pos.y + 2, 4, 4};
            renderer.drawRect(shine, {255, 255, 150, 255}, true);
            // Sparkle particles (occasional)
            if (rand() % 60 == 0) {
                particles_.emit(
                    {t.position.x + 10, t.position.y + 10},
                    {randFloat(-20, 20), randFloat(-30, -10)},
                    2, {255, 255, 100, 255}, {255, 200, 0, 0}, 0.5f);
            }
        });
    }

    // ----- Particle emitter system -----
    void updateParticleEmitters(float dt) {
        world_.each<Transform, ParticleEmitter>([&](Entity e, Transform& t, ParticleEmitter& pe) {
            if (!pe.active) return;
            pe.emitTimer += dt;
            float interval = 1.0f / pe.emitRate;
            while (pe.emitTimer >= interval) {
                pe.emitTimer -= interval;
                float angle = (pe.angle + randFloat(-pe.spread / 2, pe.spread / 2)) * 3.14159f / 180.0f;
                float spd = pe.speed + randFloat(-pe.speedVariance, pe.speedVariance);
                Vec2 vel = {std::cos(angle) * spd, std::sin(angle) * spd};
                particles_.emit(t.position + pe.offset, vel, pe.particleSize,
                               pe.startColor, pe.endColor, pe.lifetime);
            }
        });
    }

    // ----- Collision handling -----
    void handleCollision(const CollisionInfo& info) {
        Entity a = info.entityA;
        Entity b = info.entityB;

        auto getTag = [&](Entity e) -> std::string {
            if (world_.hasComponent<BoxCollider>(e))
                return world_.getComponent<BoxCollider>(e).tag;
            return "";
        };

        std::string tagA = getTag(a);
        std::string tagB = getTag(b);

        // Bullet hits enemy
        auto bulletHitsEnemy = [&](Entity bullet, Entity enemy) {
            auto& et = world_.getComponent<Transform>(enemy);
            // Death particles
            for (int i = 0; i < 15; i++) {
                float angle = randFloat(0, 6.28f);
                Vec2 pv = {std::cos(angle) * randFloat(40, 120), std::sin(angle) * randFloat(40, 120)};
                particles_.emit(et.position + Vec2{10, 10}, pv, 4,
                               {255, 80, 40, 255}, {80, 20, 10, 0}, randFloat(0.3f, 0.8f));
            }
            score_ += 10;
            toDestroy_.push_back(bullet);
            toDestroy_.push_back(enemy);
        };

        if (tagA == "bullet" && tagB == "enemy") bulletHitsEnemy(a, b);
        else if (tagA == "enemy" && tagB == "bullet") bulletHitsEnemy(b, a);

        // Player collects coin
        auto playerGetsCoin = [&](Entity player, Entity coin) {
            auto& ct = world_.getComponent<Transform>(coin);
            for (int i = 0; i < 10; i++) {
                float angle = randFloat(0, 6.28f);
                Vec2 pv = {std::cos(angle) * randFloat(30, 80), std::sin(angle) * randFloat(30, 80)};
                particles_.emit(ct.position + Vec2{6, 6}, pv, 3,
                               {255, 255, 0, 255}, {255, 150, 0, 0}, randFloat(0.2f, 0.6f));
            }
            score_ += 25;
            toDestroy_.push_back(coin);
        };

        if (tagA == "player" && tagB == "coin") playerGetsCoin(a, b);
        else if (tagA == "coin" && tagB == "player") playerGetsCoin(b, a);

        // Enemy hits player
        auto enemyHitsPlayer = [&](Entity enemy, Entity player) {
            playerHealth_--;
            // Knockback particles
            auto& pt = world_.getComponent<Transform>(player);
            for (int i = 0; i < 8; i++) {
                float angle = randFloat(0, 6.28f);
                Vec2 pv = {std::cos(angle) * randFloat(50, 100), std::sin(angle) * randFloat(50, 100)};
                particles_.emit(pt.position + Vec2{12, 12}, pv, 3,
                               {255, 100, 100, 255}, {150, 0, 0, 0}, randFloat(0.2f, 0.5f));
            }
            toDestroy_.push_back(enemy);
            if (playerHealth_ <= 0) {
                gameOver_ = true;
                // Big explosion
                for (int i = 0; i < 40; i++) {
                    float angle = randFloat(0, 6.28f);
                    Vec2 pv = {std::cos(angle) * randFloat(50, 200), std::sin(angle) * randFloat(50, 200)};
                    particles_.emit(pt.position + Vec2{12, 12}, pv, 5,
                                   {100, 150, 255, 255}, {20, 40, 100, 0}, randFloat(0.5f, 1.5f));
                }
            }
        };

        if (tagA == "enemy" && tagB == "player") enemyHitsPlayer(a, b);
        else if (tagA == "player" && tagB == "enemy") enemyHitsPlayer(b, a);
    }

    void cleanupEntities() {
        // Remove duplicates
        std::sort(toDestroy_.begin(), toDestroy_.end());
        toDestroy_.erase(std::unique(toDestroy_.begin(), toDestroy_.end()), toDestroy_.end());

        for (Entity e : toDestroy_) {
            if (world_.isAlive(e)) {
                world_.destroyEntity(e);
            }
        }
        toDestroy_.clear();
    }

    // ----- HUD -----
    void renderHUD(Renderer& renderer) {
        // Save camera and draw HUD in screen space
        Vec2 cam = renderer.getCamera();
        renderer.setCamera({0, 0});

        // Semi-transparent HUD background
        Rect hudBg = {0, 0, static_cast<float>(SCREEN_W), 40};
        renderer.drawRect(hudBg, {0, 0, 0, 150}, true);

        // Score
        std::string scoreText = "SCORE: " + std::to_string(score_);
        renderer.drawText(scoreText, {10, 12}, {255, 255, 255, 255}, 2);

        // Health
        std::string healthLabel = "HP: ";
        renderer.drawText(healthLabel, {250, 12}, {255, 255, 255, 255}, 2);
        for (int i = 0; i < playerHealth_; i++) {
            Rect heart = {310.0f + i * 22.0f, 10, 16, 16};
            renderer.drawRect(heart, {255, 50, 50, 255}, true);
        }

        // Level
        std::string levelText = "LEVEL: " + std::to_string(level_);
        renderer.drawText(levelText, {550, 12}, {255, 255, 100, 255}, 2);

        // FPS
        std::string fpsText = "FPS: " + std::to_string(static_cast<int>(engine_->getFPS()));
        renderer.drawText(fpsText, {SCREEN_W - 120.0f, 12}, {150, 150, 150, 255}, 2);

        // Controls hint
        renderer.drawText("WASD:MOVE  SPACE:SHOOT  ARROWS:AIM", {10, static_cast<float>(SCREEN_H - 20)},
                          {100, 100, 100, 255}, 1);

        if (gameOver_) {
            // Darken screen
            Rect overlay = {0, 0, static_cast<float>(SCREEN_W), static_cast<float>(SCREEN_H)};
            renderer.drawRect(overlay, {0, 0, 0, 180}, true);

            renderer.drawText("GAME OVER", {SCREEN_W / 2.0f - 80, SCREEN_H / 2.0f - 30},
                              {255, 50, 50, 255}, 4);
            std::string finalScore = "FINAL SCORE: " + std::to_string(score_);
            renderer.drawText(finalScore, {SCREEN_W / 2.0f - 100, SCREEN_H / 2.0f + 30},
                              {255, 255, 255, 255}, 2);
            renderer.drawText("PRESS R TO RESTART", {SCREEN_W / 2.0f - 90, SCREEN_H / 2.0f + 70},
                              {200, 200, 200, 255}, 2);
        }

        renderer.setCamera(cam);
    }
};

// =====================================================================
// Title Scene
// =====================================================================
class TitleScene : public Scene {
public:
    void onEnter() override {
        timer_ = 0;
    }

    void update(float dt) override {
        timer_ += dt;
        if (Input::instance().isKeyPressed(SDL_SCANCODE_RETURN) ||
            Input::instance().isKeyPressed(SDL_SCANCODE_SPACE)) {
            engine_->getSceneManager().switchTo("game");
        }

        // Ambient particles
        if (rand() % 5 == 0) {
            particles_.emit(
                {randFloat(0, SCREEN_W), randFloat(0, SCREEN_H)},
                {randFloat(-20, 20), randFloat(-40, -10)},
                3, {50, 80, 150, 200}, {20, 40, 80, 0}, randFloat(1.0f, 3.0f));
        }
        particles_.update(dt);
    }

    void render(Renderer& renderer) override {
        // Background
        Rect bg = {0, 0, static_cast<float>(SCREEN_W), static_cast<float>(SCREEN_H)};
        renderer.drawRect(bg, {15, 15, 25, 255}, true);

        renderer.setCamera({0, 0});
        particles_.render(renderer);

        // Title
        float bounce = std::sin(timer_ * 2.0f) * 5.0f;
        renderer.drawText("DUNGEON BLASTER", {SCREEN_W / 2.0f - 180, 150 + bounce},
                          {50, 150, 255, 255}, 4);

        // Subtitle
        renderer.drawText("A 2D ENGINE DEMO", {SCREEN_W / 2.0f - 100, 220},
                          {150, 150, 200, 255}, 2);

        // Features list
        float y = 300;
        Color fc = {120, 120, 160, 255};
        renderer.drawText("ENGINE FEATURES:", {SCREEN_W / 2.0f - 90, y}, {200, 200, 255, 255}, 2);
        renderer.drawText("- ENTITY COMPONENT SYSTEM", {SCREEN_W / 2.0f - 130, y + 30}, fc, 1);
        renderer.drawText("- AABB + SAT COLLISION DETECTION", {SCREEN_W / 2.0f - 130, y + 45}, fc, 1);
        renderer.drawText("- TILE MAP WITH COLLISION", {SCREEN_W / 2.0f - 130, y + 60}, fc, 1);
        renderer.drawText("- FIXED TIMESTEP GAME LOOP", {SCREEN_W / 2.0f - 130, y + 75}, fc, 1);
        renderer.drawText("- PARTICLE SYSTEM", {SCREEN_W / 2.0f - 130, y + 90}, fc, 1);
        renderer.drawText("- SCENE MANAGEMENT", {SCREEN_W / 2.0f - 130, y + 105}, fc, 1);
        renderer.drawText("- INPUT HANDLING", {SCREEN_W / 2.0f - 130, y + 120}, fc, 1);
        renderer.drawText("- CAMERA SYSTEM", {SCREEN_W / 2.0f - 130, y + 135}, fc, 1);

        // Blink prompt
        if (static_cast<int>(timer_ * 2) % 2 == 0) {
            renderer.drawText("PRESS ENTER TO START", {SCREEN_W / 2.0f - 110, SCREEN_H - 80},
                              {255, 255, 255, 255}, 2);
        }
    }

private:
    float timer_ = 0;
};

// =====================================================================
// Main
// =====================================================================
int main(int argc, char* argv[]) {
    Engine::Engine engine;

    EngineConfig config;
    config.title = "Dungeon Blaster - 2D Engine Demo";
    config.windowWidth = SCREEN_W;
    config.windowHeight = SCREEN_H;

    if (!engine.init(config)) {
        return 1;
    }

    auto titleScene = std::make_shared<TitleScene>();
    auto gameScene = std::make_shared<GameScene>();

    titleScene->setEngine(&engine);
    gameScene->setEngine(&engine);

    engine.getSceneManager().addScene("title", titleScene);
    engine.getSceneManager().addScene("game", gameScene);
    engine.getSceneManager().switchTo("title");

    engine.run();
    return 0;
}
