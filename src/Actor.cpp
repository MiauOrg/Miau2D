#include "Actor.hpp"

#include "Movement.hpp"
#include "Physics/PhysicsWorld.hpp"
#include "Scene.hpp"

#include <cmath>
#include <string>

void Actor::Update(Scene& scene, float deltaTime) {
    Movement::Update(*this, deltaTime, &scene.GetPhysics());

    const bool moving = (mover.mode == MovementMode::Grid)
        ? mover.stepActive
        : !Movement::IsZero(velocity.value);

    if (mover.mode == MovementMode::Grid && mover.stepActive) {
        facingDirection = mover.stepDirection;
    } else if (mover.mode == MovementMode::Continuous && !Movement::IsZero(velocity.value)) {
        facingDirection = Movement::DominantDirection(velocity.value);
    }

    const char* direction = "down";
    if (facingDirection.y < 0.0f) {
        direction = "up";
    } else if (facingDirection.x < 0.0f) {
        direction = "left";
    } else if (facingDirection.x > 0.0f) {
        direction = "right";
    }

    animation.Play(std::string(moving ? "walk_" : "idle_") + direction);
    SpriteObject::Update(scene, deltaTime);
}

AABB Actor::GetCollider() const {
    return AABB{
        transform.position.x + collider.offset.x,
        transform.position.y + collider.offset.y,
        collider.size.x,
        collider.size.y
    };
}

AABB Actor::GetColliderAtCell(Vector2 cell) const {
    return AABB{
        mover.gridOrigin.x + cell.x * mover.tileSize.x +
            (mover.tileSize.x - collider.size.x) * 0.5f,
        mover.gridOrigin.y + cell.y * mover.tileSize.y +
            (mover.tileSize.y - collider.size.y) * 0.5f,
        collider.size.x,
        collider.size.y
    };
}

Vector2 Actor::GetCell() const {
    const float width = (mover.tileSize.x > 0.0f) ? mover.tileSize.x : 1.0f;
    const float height = (mover.tileSize.y > 0.0f) ? mover.tileSize.y : 1.0f;

    const Vector2 center = GetCollider().Center();
    return {
        std::floor((center.x - mover.gridOrigin.x) / width),
        std::floor((center.y - mover.gridOrigin.y) / height)
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
