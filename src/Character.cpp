#include "Character.hpp"

#include "Physics/PhysicsWorld.hpp"

#include <raymath.h>

#include <algorithm>
#include <cmath>

namespace {

constexpr int MaxSnapSearchRadius = 8;

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

}  // namespace

Vector2 Character::GetCell() const {
    const float width = (tileSize.x > 0.0f) ? tileSize.x : 1.0f;
    const float height = (tileSize.y > 0.0f) ? tileSize.y : 1.0f;

    return {
        std::floor((position.x + colliderOffset.x) / width + 0.5f),
        std::floor((position.y + colliderOffset.y) / height + 0.5f)
    };
}

void Character::SetMovementMode(MovementMode mode, const PhysicsWorld* world) {
    if (mode == movementMode) {
        return;
    }

    movementMode = mode;
    velocity = { 0.0f, 0.0f };
    gridBufferedDirection = { 0.0f, 0.0f };
    gridStepActive = false;
    gridStepElapsed = 0.0f;

    if (movementMode == MovementMode::Grid) {
        SnapToGrid(world);
    }
}

void Character::SetTileSize(Vector2 size) {
    if (size.x > 0.0f && size.y > 0.0f) {
        tileSize = size;
    }
}

void Character::SetGridStepDuration(float seconds) {
    gridStepDuration = std::max(seconds, 0.01f);
}

void Character::SnapToGrid(const PhysicsWorld* world) {
    const Vector2 nearest = GetCell();

    for (int radius = 0; radius <= MaxSnapSearchRadius; ++radius) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (std::max(std::abs(dx), std::abs(dy)) != radius) {
                    continue;
                }

                const Vector2 cell { nearest.x + static_cast<float>(dx), nearest.y + static_cast<float>(dy) };
                if (CanOccupyCell(cell, world)) {
                    position = CellToPosition(cell);
                    gridStepActive = false;
                    gridStepElapsed = 0.0f;
                    gridBufferedDirection = { 0.0f, 0.0f };
                    return;
                }
            }
        }
    }

    // No free cell nearby. Allign anyway
    position = CellToPosition(nearest);
    gridStepActive = false;
    gridStepElapsed = 0.0f;
    gridBufferedDirection = { 0.0f, 0.0f };
}

void Character::Move(Vector2 direction) {
    moveIntent = direction;

    if (movementMode == MovementMode::Grid) {
        // During a step remember a different direction so the next step can be buffered
        if (gridStepActive) {
            const Vector2 dominant = DominantDirection(direction);
            if (!IsZero(dominant) && !SameDirection(dominant, gridStepDirection)) {
                gridBufferedDirection = dominant;
            }
        }
        return;
    }

    // Normalize so diagonal movement isnt faster.
    const float length = std::sqrt(
        direction.x * direction.x +
        direction.y * direction.y
    );

    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
    }

    velocity = {
        direction.x * speed,
        direction.y * speed
    };
}

void Character::Update(float deltaTime, const PhysicsWorld* world) {
    if (movementMode == MovementMode::Grid) {
        UpdateGrid(deltaTime, world);
    } else {
        UpdateContinuous(deltaTime, world);
    }
}

void Character::UpdateContinuous(float deltaTime, const PhysicsWorld* world) {
    const Vector2 displacement = { velocity.x * deltaTime, velocity.y * deltaTime };

    if (world != nullptr) {
        const Vector2 resolved = world->MoveAndSlide(GetCollider(), displacement);
        position.x += resolved.x;
        position.y += resolved.y;
    } else {
        position.x += displacement.x;
        position.y += displacement.y;
    }
}

void Character::UpdateGrid(float deltaTime, const PhysicsWorld* world) {
    if (gridStepActive) {
        gridStepElapsed += deltaTime;
        const float duration = (gridStepDuration > 0.0f) ? gridStepDuration : 0.01f;
        const float t = Clamp(gridStepElapsed / duration, 0.0f, 1.0f);

        position = Vector2Lerp(gridStepStart, gridStepTarget, t);

        if (t >= 1.0f) {
            position = gridStepTarget;
            gridStepActive = false;
            gridStepElapsed = 0.0f;
        }
    }

    if (gridStepActive) {
        return;
    }

    Vector2 direction = gridBufferedDirection;
    gridBufferedDirection = { 0.0f, 0.0f };

    if (IsZero(direction)) {
        direction = DominantDirection(moveIntent);
    }

    if (!IsZero(direction)) {
        TryStartGridStep(direction, world);
    }
}

void Character::TryStartGridStep(Vector2 direction, const PhysicsWorld* world) {
    const Vector2 cell = GetCell();
    const Vector2 targetCell { cell.x + direction.x, cell.y + direction.y };

    if (!CanOccupyCell(targetCell, world)) {
        return;
    }

    gridStepStart = CellToPosition(cell);
    gridStepTarget = CellToPosition(targetCell);
    gridStepDirection = direction;
    gridStepElapsed = 0.0f;
    gridStepActive = true;
    position = gridStepStart;
}

bool Character::CanOccupyCell(Vector2 cell, const PhysicsWorld* world) const {
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

Vector2 Character::CellToPosition(Vector2 cell) const {
    return {
        cell.x * tileSize.x - colliderOffset.x,
        cell.y * tileSize.y - colliderOffset.y
    };
}
