#pragma once
#include "Math.h"
#include "ECS.h"
#include "Components.h"
#include <vector>
#include <functional>

namespace Engine {

struct CollisionInfo {
    Entity entityA;
    Entity entityB;
    Vec2 normal;
    float depth;
    bool isTrigger;
};

class CollisionSystem {
public:
    using CollisionCallback = std::function<void(const CollisionInfo&)>;

    void setCallback(CollisionCallback cb) { callback_ = cb; }

    void update(World& world);

    // AABB collision test
    static bool testAABB(const Vec2& posA, const Vec2& sizeA,
                         const Vec2& posB, const Vec2& sizeB,
                         Vec2& normal, float& depth);

    // SAT collision test for convex polygons
    static bool testSAT(const std::vector<Vec2>& polyA,
                        const std::vector<Vec2>& polyB,
                        Vec2& normal, float& depth);

    // Helper: get world-space vertices from polygon collider + transform
    static std::vector<Vec2> getWorldVertices(const PolygonCollider& poly, const Transform& transform);

    // Helper: convert AABB to polygon vertices
    static std::vector<Vec2> aabbToVertices(const Vec2& pos, const Vec2& size);

private:
    void resolveCollision(World& world, const CollisionInfo& info);
    CollisionCallback callback_;
};

} // namespace Engine
