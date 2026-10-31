#include "Physics/PhysicsWorld.hpp"

#include <algorithm>
#include <cmath>

void PhysicsWorld::AddStaticCollider(const AABB& collider) {
    staticColliders_.push_back(collider);
}

void PhysicsWorld::AddStaticCollider(const Rectangle& rect) {
    staticColliders_.emplace_back(rect);
}

void PhysicsWorld::AddStaticColliders(const std::vector<AABB>& colliders) {
    staticColliders_.insert(staticColliders_.end(), colliders.begin(), colliders.end());
}

void PhysicsWorld::ClearStaticColliders() {
    staticColliders_.clear();
}

void PhysicsWorld::SetWorldBounds(const Rectangle& bounds) {
    worldBounds_ = AABB(bounds);
}

void PhysicsWorld::ClearWorldBounds() {
    worldBounds_.reset();
}

bool PhysicsWorld::CheckCollision(const AABB& box) const {
    for (const auto& collider : staticColliders_) {
        if (box.Intersects(collider)) {
            return true;
        }
    }
    return false;
}

std::vector<AABB> PhysicsWorld::Query(const AABB& area) const {
    std::vector<AABB> results;
    for (const auto& collider : staticColliders_) {
        if (area.Intersects(collider)) {
            results.push_back(collider);
        }
    }
    return results;
}

bool PhysicsWorld::Raycast(Vector2 origin, Vector2 direction, float maxDistance, RaycastHit& outHit) const {
    bool hitAny = false;
    float closestTime = maxDistance;
    RaycastHit closestHit{};

    for (const auto& collider : staticColliders_) {
        RaycastHit hit;
        if (collider.Raycast(origin, direction, maxDistance, hit)) {
            if (hit.time < closestTime) {
                closestTime = hit.time;
                closestHit = hit;
                hitAny = true;
            }
        }
    }

    if (hitAny) {
        outHit = closestHit;
    }
    return hitAny;
}

Vector2 PhysicsWorld::ResolveOverlap(const AABB& box) const {
    AABB current = box;
    Vector2 totalDisplacement{ 0.0f, 0.0f };

    constexpr int maxIterations = 4;
    for (int iter = 0; iter < maxIterations; ++iter) {
        bool resolvedAny = false;
        for (const auto& collider : staticColliders_) {
            Vector2 mtv{ 0.0f, 0.0f };
            if (current.ResolveCollision(collider, mtv)) {
                current.position.x += mtv.x;
                current.position.y += mtv.y;
                totalDisplacement.x += mtv.x;
                totalDisplacement.y += mtv.y;
                resolvedAny = true;
            }
        }
        if (!resolvedAny) break;
    }

    return totalDisplacement;
}

Vector2 PhysicsWorld::MoveAndSlide(const AABB& box, Vector2 displacement) const {
    AABB current = box;
    const Vector2 initialPush = ResolveOverlap(current);
    current.position.x += initialPush.x;
    current.position.y += initialPush.y;

    if (displacement.x > 0.0f) {
        float newX = current.position.x + displacement.x;
        for (const auto& collider : staticColliders_) {
            if (current.Top() < collider.Bottom() && current.Bottom() > collider.Top()) {
                if (collider.Left() >= current.Right() - 0.001f && collider.Left() < newX + current.size.x) {
                    newX = std::min(newX, collider.Left() - current.size.x);
                }
            }
        }
        current.position.x = newX;
    } else if (displacement.x < 0.0f) {
        float newX = current.position.x + displacement.x;
        for (const auto& collider : staticColliders_) {
            if (current.Top() < collider.Bottom() && current.Bottom() > collider.Top()) {
                if (collider.Right() <= current.Left() + 0.001f && collider.Right() > newX) {
                    newX = std::max(newX, collider.Right());
                }
            }
        }
        current.position.x = newX;
    }

    if (displacement.y > 0.0f) {
        float newY = current.position.y + displacement.y;
        for (const auto& collider : staticColliders_) {
            if (current.Left() < collider.Right() && current.Right() > collider.Left()) {
                if (collider.Top() >= current.Bottom() - 0.001f && collider.Top() < newY + current.size.y) {
                    newY = std::min(newY, collider.Top() - current.size.y);
                }
            }
        }
        current.position.y = newY;
    } else if (displacement.y < 0.0f) {
        float newY = current.position.y + displacement.y;
        for (const auto& collider : staticColliders_) {
            if (current.Left() < collider.Right() && current.Right() > collider.Left()) {
                if (collider.Bottom() <= current.Top() + 0.001f && collider.Bottom() > newY) {
                    newY = std::max(newY, collider.Bottom());
                }
            }
        }
        current.position.y = newY;
    }

    if (worldBounds_.has_value()) {
        current.position.x = std::clamp(
            current.position.x,
            worldBounds_->Left(),
            worldBounds_->Right() - current.size.x
        );
        current.position.y = std::clamp(
            current.position.y,
            worldBounds_->Top(),
            worldBounds_->Bottom() - current.size.y
        );
    }

    return {
        current.position.x - box.position.x,
        current.position.y - box.position.y
    };
}

void PhysicsWorld::DrawDebug(Color fillColor, Color outlineColor, float thickness) const {
    for (const auto& collider : staticColliders_) {
        collider.Draw(fillColor);
        collider.DrawLines(outlineColor, thickness);
    }

    if (worldBounds_.has_value()) {
        worldBounds_->DrawLines(BLUE, thickness + 1.0f);
    }
}
