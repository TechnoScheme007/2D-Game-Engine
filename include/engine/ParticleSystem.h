#pragma once
#include "Math.h"
#include "Renderer.h"
#include <vector>

namespace Engine {

struct Particle {
    Vec2 position;
    Vec2 velocity;
    Color startColor, endColor;
    float lifetime;
    float maxLifetime;
    int size;
    bool alive = false;
};

class ParticlePool {
public:
    ParticlePool(int maxParticles = 500);

    void emit(const Vec2& pos, const Vec2& velocity, int size,
              const Color& startColor, const Color& endColor, float lifetime);

    void update(float dt);
    void render(Renderer& renderer, const Vec2& cameraOffset = {}) const;
    void clear();

    int activeCount() const;

private:
    std::vector<Particle> particles_;
    int nextIndex_ = 0;
};

} // namespace Engine
