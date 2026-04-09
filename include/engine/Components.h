#pragma once
#include "Math.h"
#include <string>
#include <vector>
#include <SDL2/SDL.h>

namespace Engine {

struct Transform {
    Vec2 position;
    Vec2 scale = {1.0f, 1.0f};
    float rotation = 0.0f; // degrees
};

struct Velocity {
    Vec2 velocity;
    float drag = 0.0f;
};

struct Sprite {
    std::string textureId;
    SDL_Rect srcRect = {0, 0, 0, 0};  // source rect in texture (0 = full)
    int width = 32;
    int height = 32;
    int zOrder = 0;
    bool flipX = false;
    bool flipY = false;
    Color tint;
};

struct Animation {
    int frameCount = 1;
    int currentFrame = 0;
    float frameTime = 0.1f;
    float elapsed = 0.0f;
    int frameWidth = 32;
    int frameHeight = 32;
    int startX = 0;
    int startY = 0;
    bool loop = true;
    bool playing = true;
};

struct BoxCollider {
    Vec2 offset;
    Vec2 size = {32.0f, 32.0f};
    bool isTrigger = false;
    bool isStatic = false;
    std::string tag;
};

// For SAT collision: convex polygon collider
struct PolygonCollider {
    std::vector<Vec2> vertices; // relative to transform position
    bool isTrigger = false;
    bool isStatic = false;
    std::string tag;
};

struct RigidBody {
    Vec2 acceleration;
    float mass = 1.0f;
    float restitution = 0.0f; // bounciness
    float friction = 0.5f;
    bool useGravity = false;
    bool isKinematic = false;
};

struct Health {
    int current = 100;
    int max = 100;
    float invincibleTimer = 0.0f;
};

struct Tag {
    std::string name;
};

struct ParticleEmitter {
    std::string textureId;
    Vec2 offset;
    float emitRate = 10.0f;     // particles per second
    float emitTimer = 0.0f;
    float lifetime = 1.0f;      // particle lifetime
    float speed = 100.0f;
    float speedVariance = 20.0f;
    float angle = 0.0f;         // emission direction (degrees)
    float spread = 360.0f;      // emission cone (degrees)
    int particleSize = 4;
    Color startColor = {255, 255, 100, 255};
    Color endColor = {255, 50, 0, 0};
    bool active = true;
    int maxParticles = 100;
};

struct CameraFollow {
    Vec2 offset;
    float smoothing = 5.0f;
};

struct Lifetime {
    float remaining = 5.0f;
};

} // namespace Engine
