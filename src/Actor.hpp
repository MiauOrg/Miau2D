#pragma once

#include "GameObject.hpp"
#include "Physics/AABB.hpp"

class PhysicsWorld;

enum class MovementMode {
    Continuous,
    Grid,
};

struct Collider {
    Vector2 size{ 0.0f, 0.0f };
};

struct Velocity {
    Vector2 value{ 0.0f, 0.0f };
    float speed = 100.0f;
};

struct GridMover {
    MovementMode mode = MovementMode::Grid;
    Vector2 gridOrigin{ 0.0f, 0.0f };
    Vector2 tileSize{ 16.0f, 16.0f };
    float stepDuration = 0.16f;

    Vector2 intent{ 0.0f, 0.0f };
    Vector2 stepStart{ 0.0f, 0.0f };
    Vector2 stepTarget{ 0.0f, 0.0f };
    Vector2 stepDirection{ 0.0f, 0.0f };
    Vector2 bufferedDirection{ 0.0f, 0.0f };
    float stepElapsed = 0.0f;
    bool stepActive = false;
};

class Actor : public SpriteObject {
public:
    Collider collider;
    Velocity velocity;
    GridMover mover;
    Vector2 facingDirection{ 0.0f, 1.0f };

    void Update(Scene& scene, float deltaTime) override;
    void DrawDebug() const override;

    [[nodiscard]] AABB GetCollider() const;
    [[nodiscard]] AABB GetColliderAtCell(Vector2 cell) const;
    [[nodiscard]] Vector2 GetCell() const;
    [[nodiscard]] bool CanOccupyCell(Vector2 cell, const PhysicsWorld* world) const;

    [[nodiscard]] Vector2 CellToPosition(Vector2 cell) const {
        return {
            mover.gridOrigin.x + cell.x * mover.tileSize.x + mover.tileSize.x * 0.5f,
            mover.gridOrigin.y + cell.y * mover.tileSize.y + mover.tileSize.y
        };
    }

    [[nodiscard]] MovementMode GetMovementMode() const { return mover.mode; }
    [[nodiscard]] bool IsGridMoving() const { return mover.stepActive; }
};
