#include "engine/ParticleSystem.h"
#include <cmath>

namespace Engine {

ParticlePool::ParticlePool(int maxParticles) {
    particles_.resize(maxParticles);
}

void ParticlePool::emit(const Vec2& pos, const Vec2& velocity, int size,
                         const Color& startColor, const Color& endColor, float lifetime) {
    Particle& p = particles_[nextIndex_];
    p.position = pos;
    p.velocity = velocity;
    p.size = size;
    p.startColor = startColor;
    p.endColor = endColor;
    p.lifetime = lifetime;
    p.maxLifetime = lifetime;
    p.alive = true;
    nextIndex_ = (nextIndex_ + 1) % particles_.size();
}

void ParticlePool::update(float dt) {
    for (auto& p : particles_) {
        if (!p.alive) continue;
        p.lifetime -= dt;
        if (p.lifetime <= 0) {
            p.alive = false;
            continue;
        }
        p.position += p.velocity * dt;
        p.velocity *= (1.0f - 2.0f * dt); // damping
    }
}

void ParticlePool::render(Renderer& renderer, const Vec2& cameraOffset) const {
    for (const auto& p : particles_) {
        if (!p.alive) continue;
        float t = 1.0f - (p.lifetime / p.maxLifetime);
        Color c = Color::lerp(p.startColor, p.endColor, t);
        Rect r = {p.position.x - p.size / 2.0f, p.position.y - p.size / 2.0f,
                  static_cast<float>(p.size), static_cast<float>(p.size)};
        renderer.drawRect(r, c, true);
    }
}

void ParticlePool::clear() {
    for (auto& p : particles_) p.alive = false;
    nextIndex_ = 0;
}

int ParticlePool::activeCount() const {
    int count = 0;
    for (const auto& p : particles_) if (p.alive) count++;
    return count;
}

} // namespace Engine
