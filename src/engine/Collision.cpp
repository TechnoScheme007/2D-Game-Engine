#include "engine/Collision.h"
#include <limits>
#include <cmath>

namespace Engine {

void CollisionSystem::update(World& world) {
    // Collect all entities with box colliders
    auto boxEntities = world.view<Transform, BoxCollider>();

    // Check all pairs
    for (size_t i = 0; i < boxEntities.size(); i++) {
        for (size_t j = i + 1; j < boxEntities.size(); j++) {
            Entity a = boxEntities[i];
            Entity b = boxEntities[j];

            auto& tA = world.getComponent<Transform>(a);
            auto& cA = world.getComponent<BoxCollider>(a);
            auto& tB = world.getComponent<Transform>(b);
            auto& cB = world.getComponent<BoxCollider>(b);

            Vec2 posA = tA.position + cA.offset;
            Vec2 posB = tB.position + cB.offset;

            Vec2 normal;
            float depth;

            if (testAABB(posA, cA.size, posB, cB.size, normal, depth)) {
                CollisionInfo info;
                info.entityA = a;
                info.entityB = b;
                info.normal = normal;
                info.depth = depth;
                info.isTrigger = cA.isTrigger || cB.isTrigger;

                if (callback_) callback_(info);

                if (!info.isTrigger) {
                    resolveCollision(world, info);
                }
            }
        }
    }

    // SAT collision for polygon colliders
    auto polyEntities = world.view<Transform, PolygonCollider>();
    for (size_t i = 0; i < polyEntities.size(); i++) {
        for (size_t j = i + 1; j < polyEntities.size(); j++) {
            Entity a = polyEntities[i];
            Entity b = polyEntities[j];

            auto& tA = world.getComponent<Transform>(a);
            auto& pA = world.getComponent<PolygonCollider>(a);
            auto& tB = world.getComponent<Transform>(b);
            auto& pB = world.getComponent<PolygonCollider>(b);

            auto vertsA = getWorldVertices(pA, tA);
            auto vertsB = getWorldVertices(pB, tB);

            Vec2 normal;
            float depth;

            if (testSAT(vertsA, vertsB, normal, depth)) {
                CollisionInfo info;
                info.entityA = a;
                info.entityB = b;
                info.normal = normal;
                info.depth = depth;
                info.isTrigger = pA.isTrigger || pB.isTrigger;

                if (callback_) callback_(info);
            }
        }
    }
}

bool CollisionSystem::testAABB(const Vec2& posA, const Vec2& sizeA,
                                const Vec2& posB, const Vec2& sizeB,
                                Vec2& normal, float& depth) {
    float overlapX = std::min(posA.x + sizeA.x, posB.x + sizeB.x) - std::max(posA.x, posB.x);
    float overlapY = std::min(posA.y + sizeA.y, posB.y + sizeB.y) - std::max(posA.y, posB.y);

    if (overlapX <= 0 || overlapY <= 0) return false;

    Vec2 centerA = {posA.x + sizeA.x / 2, posA.y + sizeA.y / 2};
    Vec2 centerB = {posB.x + sizeB.x / 2, posB.y + sizeB.y / 2};

    if (overlapX < overlapY) {
        depth = overlapX;
        normal = {centerA.x < centerB.x ? -1.0f : 1.0f, 0.0f};
    } else {
        depth = overlapY;
        normal = {0.0f, centerA.y < centerB.y ? -1.0f : 1.0f};
    }

    return true;
}

bool CollisionSystem::testSAT(const std::vector<Vec2>& polyA,
                               const std::vector<Vec2>& polyB,
                               Vec2& normal, float& depth) {
    depth = std::numeric_limits<float>::max();

    auto projectPolygon = [](const std::vector<Vec2>& poly, const Vec2& axis, float& min, float& max) {
        min = std::numeric_limits<float>::max();
        max = std::numeric_limits<float>::lowest();
        for (const auto& v : poly) {
            float proj = v.dot(axis);
            if (proj < min) min = proj;
            if (proj > max) max = proj;
        }
    };

    auto checkAxes = [&](const std::vector<Vec2>& poly) -> bool {
        for (size_t i = 0; i < poly.size(); i++) {
            Vec2 edge = poly[(i + 1) % poly.size()] - poly[i];
            Vec2 axis = edge.perp().normalized();

            float minA, maxA, minB, maxB;
            projectPolygon(polyA, axis, minA, maxA);
            projectPolygon(polyB, axis, minB, maxB);

            float overlap = std::min(maxA, maxB) - std::max(minA, minB);
            if (overlap <= 0) return false;

            if (overlap < depth) {
                depth = overlap;
                normal = axis;
            }
        }
        return true;
    };

    if (!checkAxes(polyA)) return false;
    if (!checkAxes(polyB)) return false;

    // Ensure normal points from A to B
    Vec2 centerA, centerB;
    for (const auto& v : polyA) { centerA.x += v.x; centerA.y += v.y; }
    centerA = centerA / static_cast<float>(polyA.size());
    for (const auto& v : polyB) { centerB.x += v.x; centerB.y += v.y; }
    centerB = centerB / static_cast<float>(polyB.size());

    Vec2 dir = centerB - centerA;
    if (dir.dot(normal) < 0) {
        normal = normal * -1.0f;
    }

    return true;
}

std::vector<Vec2> CollisionSystem::getWorldVertices(const PolygonCollider& poly, const Transform& transform) {
    std::vector<Vec2> result;
    result.reserve(poly.vertices.size());

    float rad = transform.rotation * 3.14159265f / 180.0f;
    float cosR = std::cos(rad);
    float sinR = std::sin(rad);

    for (const auto& v : poly.vertices) {
        Vec2 scaled = {v.x * transform.scale.x, v.y * transform.scale.y};
        Vec2 rotated = {
            scaled.x * cosR - scaled.y * sinR,
            scaled.x * sinR + scaled.y * cosR
        };
        result.push_back(rotated + transform.position);
    }
    return result;
}

std::vector<Vec2> CollisionSystem::aabbToVertices(const Vec2& pos, const Vec2& size) {
    return {
        pos,
        {pos.x + size.x, pos.y},
        {pos.x + size.x, pos.y + size.y},
        {pos.x, pos.y + size.y}
    };
}

void CollisionSystem::resolveCollision(World& world, const CollisionInfo& info) {
    auto& cA = world.getComponent<BoxCollider>(info.entityA);
    auto& cB = world.getComponent<BoxCollider>(info.entityB);

    bool aStatic = cA.isStatic;
    bool bStatic = cB.isStatic;

    if (aStatic && bStatic) return;

    auto& tA = world.getComponent<Transform>(info.entityA);
    auto& tB = world.getComponent<Transform>(info.entityB);

    if (aStatic) {
        tB.position += info.normal * info.depth;
    } else if (bStatic) {
        tA.position -= info.normal * info.depth;
    } else {
        tA.position -= info.normal * (info.depth * 0.5f);
        tB.position += info.normal * (info.depth * 0.5f);
    }

    // Velocity response
    if (world.hasComponent<Velocity>(info.entityA) && !aStatic) {
        auto& vA = world.getComponent<Velocity>(info.entityA);
        float dot = vA.velocity.dot(info.normal);
        if (dot > 0) vA.velocity -= info.normal * dot;
    }
    if (world.hasComponent<Velocity>(info.entityB) && !bStatic) {
        auto& vB = world.getComponent<Velocity>(info.entityB);
        float dot = vB.velocity.dot(info.normal);
        if (dot < 0) vB.velocity -= info.normal * dot;
    }
}

} // namespace Engine
