#pragma once

#include "Physics/AABB.hpp"

#include <vector>
#include <optional>

class PhysicsWorld {
public:
    PhysicsWorld() = default;

    // Static colliders
    void AddStaticCollider(const AABB& collider);
    void AddStaticCollider(const Rectangle& rect);
    void AddStaticColliders(const std::vector<AABB>& colliders);
    void ClearStaticColliders();
    const std::vector<AABB>& GetStaticColliders() const { return staticColliders_; }
    std::size_t GetColliderCount() const { return staticColliders_.size(); }

    // World boundary limits
    void SetWorldBounds(const Rectangle& bounds);
    void ClearWorldBounds();
    bool HasWorldBounds() const { return worldBounds_.has_value(); }
    std::optional<AABB> GetWorldBounds() const { return worldBounds_; }

    // Spatial & overlap queries
    bool CheckCollision(const AABB& box) const;
    std::vector<AABB> Query(const AABB& area) const;
    bool Raycast(Vector2 origin, Vector2 direction, float maxDistance, RaycastHit& outHit) const;

    // Movement resolution:
    // Moves an AABB by displacement, sliding along obstacles.
    // Returns the actual resolved displacement.
    Vector2 MoveAndSlide(const AABB& box, Vector2 displacement) const;

    // Resolves penetration if an AABB is overlapping any static geometry
    Vector2 ResolveOverlap(const AABB& box) const;

    // Debug visualization
    void DrawDebug(Color fillColor = { 255, 0, 0, 40 }, Color outlineColor = RED, float thickness = 1.0f) const;

private:
    std::vector<AABB> staticColliders_;
    std::optional<AABB> worldBounds_;
};