#include "Movement.hpp"

#include "Physics/PhysicsWorld.hpp"

#include <raymath.h>

#include <algorithm>
#include <cmath>

namespace {

constexpr int MaxSnapSearchRadius = 8;

}  // namespace

namespace Movement {

Vector2 DominantDirection(Vector2 direction) {
    const float absX = std::fabs(direction.x);
    const float absY = std::fabs(direction.y);

    if (absX == 0.0f && absY == 0.0f) {
        return { 0.0f, 0.0f };
    }

    // Grid movement is 4 directional so pick a single axis.
    if (absX >= absY) {
        return { (direction.x > 0.0f) ? 1.0f : -1.0f, 0.0f };
    }
    return { 0.0f, (direction.y > 0.0f) ? 1.0f : -1.0f };
}

bool IsZero(Vector2 value) {
    return value.x == 0.0f && value.y == 0.0f;
}

bool SameDirection(Vector2 a, Vector2 b) {
    return a.x == b.x && a.y == b.y;
}

void SetTileSize(Actor& actor, Vector2 size) {
    if (size.x > 0.0f && size.y > 0.0f) {
        actor.mover.tileSize = size;
    }
}

void SetStepDuration(Actor& actor, float seconds) {
    actor.mover.stepDuration = std::max(seconds, 0.01f);
}

void SnapToGrid(Actor& actor, const PhysicsWorld* world) {
    const Vector2 nearest = actor.GetCell();
    auto& mover = actor.mover;

    for (int radius = 0; radius <= MaxSnapSearchRadius; ++radius) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (std::max(std::abs(dx), std::abs(dy)) != radius) {
                    continue;
                }

                const Vector2 cell {
                    nearest.x + static_cast<float>(dx),
                    nearest.y + static_cast<float>(dy)
                };

                if (actor.CanOccupyCell(cell, world)) {
                    actor.transform.position = actor.CellToPosition(cell);
                    mover.stepActive = false;
                    mover.stepElapsed = 0.0f;
                    mover.bufferedDirection = { 0.0f, 0.0f };
                    return;
                }
            }
        }
    }

    // No free cell nearby. Align anyway.
    actor.transform.position = actor.CellToPosition(nearest);
    mover.stepActive = false;
    mover.stepElapsed = 0.0f;
    mover.bufferedDirection = { 0.0f, 0.0f };
}

void SetMovementMode(Actor& actor, MovementMode mode, const PhysicsWorld* world) {
    auto& mover = actor.mover;
    if (mode == mover.mode) {
        return;
    }

    mover.mode = mode;
    actor.velocity.value = { 0.0f, 0.0f };
    mover.bufferedDirection = { 0.0f, 0.0f };
    mover.stepActive = false;
    mover.stepElapsed = 0.0f;

    if (mover.mode == MovementMode::Grid) {
        SnapToGrid(actor, world);
    }
}

void Move(Actor& actor, Vector2 direction) {
    auto& mover = actor.mover;
    mover.intent = direction;

    if (mover.mode == MovementMode::Grid) {
        // During a step remember a different direction so the next step can be buffered.
        if (mover.stepActive) {
            const Vector2 dominant = DominantDirection(direction);
            if (!IsZero(dominant) && !SameDirection(dominant, mover.stepDirection)) {
                mover.bufferedDirection = dominant;
            }
        }
        return;
    }

    // Normalize so diagonal movement isn't faster.
    const float length = std::sqrt(
        direction.x * direction.x +
        direction.y * direction.y
    );

    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
    }

    actor.velocity.value = {
        direction.x * actor.velocity.speed,
        direction.y * actor.velocity.speed
    };
}

namespace {

void TryStartGridStep(Actor& actor, Vector2 direction, const PhysicsWorld* world) {
    auto& mover = actor.mover;
    const Vector2 cell = actor.GetCell();
    const Vector2 targetCell { cell.x + direction.x, cell.y + direction.y };

    if (!actor.CanOccupyCell(targetCell, world)) {
        return;
    }

    mover.stepStart = actor.CellToPosition(cell);
    mover.stepTarget = actor.CellToPosition(targetCell);
    mover.stepDirection = direction;
    mover.stepElapsed = 0.0f;
    mover.stepActive = true;
    actor.transform.position = mover.stepStart;
}

void UpdateGrid(Actor& actor, float deltaTime, const PhysicsWorld* world) {
    auto& mover = actor.mover;

    if (mover.stepActive) {
        mover.stepElapsed += deltaTime;
        const float duration = (mover.stepDuration > 0.0f) ? mover.stepDuration : 0.01f;
        const float t = Clamp(mover.stepElapsed / duration, 0.0f, 1.0f);

        actor.transform.position = Vector2Lerp(mover.stepStart, mover.stepTarget, t);

        if (t >= 1.0f) {
            actor.transform.position = mover.stepTarget;
            mover.stepActive = false;
            mover.stepElapsed = 0.0f;
        }
    }

    if (mover.stepActive) {
        return;
    }

    Vector2 direction = mover.bufferedDirection;
    mover.bufferedDirection = { 0.0f, 0.0f };

    if (IsZero(direction)) {
        direction = DominantDirection(mover.intent);
    }

    if (!IsZero(direction)) {
        TryStartGridStep(actor, direction, world);
    }
}

void UpdateContinuous(Actor& actor, float deltaTime, const PhysicsWorld* world) {
    const Vector2 displacement = {
        actor.velocity.value.x * deltaTime,
        actor.velocity.value.y * deltaTime
    };

    if (world != nullptr) {
        const Vector2 resolved = world->MoveAndSlide(actor.GetCollider(), displacement);
        actor.transform.position.x += resolved.x;
        actor.transform.position.y += resolved.y;
    } else {
        actor.transform.position.x += displacement.x;
        actor.transform.position.y += displacement.y;
    }
}

}  // namespace

void Update(Actor& actor, float deltaTime, const PhysicsWorld* world) {
    if (actor.mover.mode == MovementMode::Grid) {
        UpdateGrid(actor, deltaTime, world);
    } else {
        UpdateContinuous(actor, deltaTime, world);
    }
}

}  // namespace Movement
