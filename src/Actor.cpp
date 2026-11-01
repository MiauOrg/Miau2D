#include "Actor.hpp"

#include "Movement.hpp"
#include "Physics/PhysicsWorld.hpp"
#include "Scene.hpp"

#include <algorithm>
#include <cmath>

void Actor::Update(Scene& scene, float deltaTime) {
    Movement::Update(*this, deltaTime, &scene.GetPhysics());
}

AABB Actor::GetCollider() const {
    return AABB{
        transform.position.x - collider.size.x * 0.5f,
        transform.position.y - collider.size.y,
        collider.size.x,
        collider.size.y
    };
}

AABB Actor::GetColliderAtCell(Vector2 cell) const {
    const float left = cell.x * mover.tileSize.x
        + (mover.tileSize.x - collider.size.x) * 0.5f;
    const float top = cell.y * mover.tileSize.y
        + (mover.tileSize.y - collider.size.y);

    return AABB{
        left,
        top,
        collider.size.x,
        collider.size.y
    };
}

Vector2 Actor::GetCell() const {
    const float width = (mover.tileSize.x > 0.0f) ? mover.tileSize.x : 1.0f;
    const float height = (mover.tileSize.y > 0.0f) ? mover.tileSize.y : 1.0f;

    return {
        std::floor(transform.position.x / width),
        std::floor((transform.position.y - 0.0001f) / height)
    };
}

bool Actor::CanOccupyCell(Vector2 cell, const PhysicsWorld* world) const {
    if (world == nullptr) {
        return true;
    }

    const AABB box = GetColliderAtCell(cell);

    if (const auto bounds = world->GetWorldBounds()) {
        if (!bounds->Contains(box)) {
            return false;
        }
    }

    return !world->CheckCollision(box);
}

void Actor::DrawDebug() const {
    GetCollider().DrawLines(GREEN, 1.5f);
}
