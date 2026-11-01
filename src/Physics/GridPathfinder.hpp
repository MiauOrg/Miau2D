#pragma once

#include "Physics/AABB.hpp"

#include <vector>

class PhysicsWorld;

struct GridCell {
    int x = 0;
    int y = 0;

    bool operator==(const GridCell&) const = default;
};

class GridPathfinder {
public:
    GridPathfinder() = default;
    GridPathfinder(const PhysicsWorld& world, const AABB& bounds, Vector2 cellSize);

    bool SetGrid(const PhysicsWorld& world, const AABB& bounds, Vector2 cellSize);
    [[nodiscard]] Vector2 GetCellSize() const { return cellSize_; }
    [[nodiscard]] int GetColumnCount() const { return columns_; }
    [[nodiscard]] int GetRowCount() const { return rows_; }
    [[nodiscard]] bool IsValidCell(GridCell cell) const;
    [[nodiscard]] GridCell WorldToCell(Vector2 worldPosition) const;
    [[nodiscard]] Vector2 CellToWorld(GridCell cell) const;
    [[nodiscard]] bool IsWalkable(GridCell cell, Vector2 actorSize) const;
    bool FindPath(GridCell start, GridCell goal, Vector2 actorSize,
                  std::vector<GridCell>& outPath) const;

private:
    const PhysicsWorld* world_ = nullptr;
    AABB bounds_{};
    Vector2 cellSize_{ 0.0f, 0.0f };
    int columns_ = 0;
    int rows_ = 0;
};
