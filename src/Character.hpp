#pragma once

#include "raylib.h"
#include "Physics/AABB.hpp"

class PhysicsWorld;

enum class MovementMode {
    Continuous,
    Grid,
};

class Character {
public:
    Vector2 position { 0.0f, 0.0f };
    Vector2 textureOffset { 0.0f, 0.0f };
    float speed = 100.0f;
    Vector2 velocity { 0.0f, 0.0f };
    Rectangle rect = {
        0.0f,   // x
        0.0f,   // y
        24.0f,  // width
        24.0f,  // height
    };

    // Collider settings relative to character position
    Vector2 colliderOffset { 4.0f, 8.0f };
    Vector2 colliderSize { 16.0f, 16.0f };

    // Grid movement
    MovementMode movementMode = MovementMode::Grid;
    Vector2 tileSize { 16.0f, 16.0f };
    float gridStepDuration = 0.16f;

    Character() = default;
    Character(Vector2 startPos) : position(startPos) {}

    AABB GetCollider() const {
        return AABB{
            position.x + colliderOffset.x,
            position.y + colliderOffset.y,
            colliderSize.x,
            colliderSize.y
        };
    }

    AABB GetColliderAtCell(Vector2 cell) const {
        return AABB{
            cell.x * tileSize.x,
            cell.y * tileSize.y,
            colliderSize.x,
            colliderSize.y
        };
    }

    Vector2 GetCell() const;

    MovementMode GetMovementMode() const { return movementMode; }
    bool IsGridMoving() const { return gridStepActive; }

    void SetMovementMode(MovementMode mode, const PhysicsWorld* world = nullptr);
    void SetTileSize(Vector2 size);
    void SetGridStepDuration(float seconds);

    void SnapToGrid(const PhysicsWorld* world = nullptr);

    void Move(Vector2 direction);
    void Update(float deltaTime, const PhysicsWorld* world = nullptr);

    void Draw(Texture2D texture) const {
        DrawCircleV(position, 8.0f, RED);
        DrawTextureRec(texture, rect, position, WHITE);
    }

    void DrawDebug(Color color = GREEN) const {
        GetCollider().DrawLines(color, 1.5f);
    }

private:
    Vector2 moveIntent { 0.0f, 0.0f };

    bool gridStepActive = false;
    Vector2 gridStepStart { 0.0f, 0.0f };
    Vector2 gridStepTarget { 0.0f, 0.0f };
    Vector2 gridStepDirection { 0.0f, 0.0f };
    Vector2 gridBufferedDirection { 0.0f, 0.0f };
    float gridStepElapsed = 0.0f;

    void UpdateContinuous(float deltaTime, const PhysicsWorld* world);
    void UpdateGrid(float deltaTime, const PhysicsWorld* world);
    void TryStartGridStep(Vector2 direction, const PhysicsWorld* world);
    [[nodiscard]] bool CanOccupyCell(Vector2 cell, const PhysicsWorld* world) const;
    [[nodiscard]] Vector2 CellToPosition(Vector2 cell) const;
};
