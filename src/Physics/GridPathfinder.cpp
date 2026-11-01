#include "Physics/GridPathfinder.hpp"

#include "Physics/PhysicsWorld.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <vector>

namespace {

struct OpenNode {
    int index = 0;
    int cost = 0;
    int heuristic = 0;
    std::size_t order = 0;
};

struct OpenNodeOrder {
    bool operator()(const OpenNode& a, const OpenNode& b) const {
        const int aScore = a.cost + a.heuristic;
        const int bScore = b.cost + b.heuristic;
        if (aScore != bScore) {
            return aScore > bScore;
        }
        if (a.heuristic != b.heuristic) {
            return a.heuristic > b.heuristic;
        }
        return a.order > b.order;
    }
};

int ManhattanDistance(GridCell a, GridCell b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

}

GridPathfinder::GridPathfinder(const PhysicsWorld& world, const AABB& bounds, Vector2 cellSize) {
    SetGrid(world, bounds, cellSize);
}

bool GridPathfinder::SetGrid(const PhysicsWorld& world, const AABB& bounds, Vector2 cellSize) {
    world_ = nullptr;
    columns_ = 0;
    rows_ = 0;

    if (!std::isfinite(bounds.position.x) || !std::isfinite(bounds.position.y) ||
        !std::isfinite(bounds.size.x) || !std::isfinite(bounds.size.y) ||
        !std::isfinite(cellSize.x) || !std::isfinite(cellSize.y) ||
        bounds.size.x <= 0.0f || bounds.size.y <= 0.0f ||
        cellSize.x <= 0.0f || cellSize.y <= 0.0f) {
        return false;
    }

    const double columns = std::ceil(static_cast<double>(bounds.size.x) / cellSize.x);
    const double rows = std::ceil(static_cast<double>(bounds.size.y) / cellSize.y);
    if (columns > std::numeric_limits<int>::max() || rows > std::numeric_limits<int>::max() ||
        columns * rows > std::numeric_limits<int>::max()) {
        return false;
    }

    bounds_ = bounds;
    cellSize_ = cellSize;
    columns_ = static_cast<int>(columns);
    rows_ = static_cast<int>(rows);
    world_ = &world;
    return true;
}

bool GridPathfinder::IsValidCell(GridCell cell) const {
    return world_ != nullptr && cell.x >= 0 && cell.y >= 0 &&
           cell.x < columns_ && cell.y < rows_;
}

GridCell GridPathfinder::WorldToCell(Vector2 worldPosition) const {
    if (world_ == nullptr || !std::isfinite(worldPosition.x) || !std::isfinite(worldPosition.y)) {
        return { -1, -1 };
    }

    return {
        static_cast<int>(std::floor((worldPosition.x - bounds_.position.x) / cellSize_.x)),
        static_cast<int>(std::floor((worldPosition.y - bounds_.position.y) / cellSize_.y))
    };
}

Vector2 GridPathfinder::CellToWorld(GridCell cell) const {
    if (!IsValidCell(cell)) {
        return { 0.0f, 0.0f };
    }

    return {
        bounds_.position.x + (static_cast<float>(cell.x) + 0.5f) * cellSize_.x,
        bounds_.position.y + (static_cast<float>(cell.y) + 0.5f) * cellSize_.y
    };
}

bool GridPathfinder::IsWalkable(GridCell cell, Vector2 actorSize) const {
    if (!IsValidCell(cell) || !std::isfinite(actorSize.x) || !std::isfinite(actorSize.y) ||
        actorSize.x <= 0.0f || actorSize.y <= 0.0f) {
        return false;
    }

    const Vector2 center = CellToWorld(cell);
    const AABB footprint(
        center.x - actorSize.x * 0.5f,
        center.y - actorSize.y * 0.5f,
        actorSize.x,
        actorSize.y);

    return bounds_.Contains(footprint) && !world_->CheckCollision(footprint);
}

bool GridPathfinder::FindPath(GridCell start, GridCell goal, Vector2 actorSize,
                              std::vector<GridCell>& outPath) const {
    outPath.clear();
    if (!IsWalkable(start, actorSize) || !IsWalkable(goal, actorSize)) {
        return false;
    }
    if (start == goal) {
        return true;
    }

    const int nodeCount = columns_ * rows_;
    const int startIndex = start.y * columns_ + start.x;
    const int goalIndex = goal.y * columns_ + goal.x;
    const int unreachable = std::numeric_limits<int>::max();
    std::vector<int> costs(static_cast<std::size_t>(nodeCount), unreachable);
    std::vector<int> parents(static_cast<std::size_t>(nodeCount), -1);
    std::vector<bool> closed(static_cast<std::size_t>(nodeCount), false);
    std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeOrder> open;

    std::size_t order = 0;
    costs[startIndex] = 0;
    open.push({ startIndex, 0, ManhattanDistance(start, goal), order++ });

    constexpr GridCell directions[] = {
        { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 }
    };

    while (!open.empty()) {
        const OpenNode node = open.top();
        open.pop();
        if (closed[node.index] || node.cost != costs[node.index]) {
            continue;
        }
        if (node.index == goalIndex) {
            int current = goalIndex;
            while (current != startIndex) {
                outPath.push_back({ current % columns_, current / columns_ });
                current = parents[current];
                if (current < 0) {
                    outPath.clear();
                    return false;
                }
            }
            std::reverse(outPath.begin(), outPath.end());
            return true;
        }

        closed[node.index] = true;
        const GridCell currentCell{ node.index % columns_, node.index / columns_ };
        for (const GridCell direction : directions) {
            const GridCell neighbor{
                currentCell.x + direction.x,
                currentCell.y + direction.y
            };
            if (!IsWalkable(neighbor, actorSize)) {
                continue;
            }

            const int neighborIndex = neighbor.y * columns_ + neighbor.x;
            if (closed[neighborIndex]) {
                continue;
            }

            const int candidateCost = costs[node.index] + 1;
            if (candidateCost >= costs[neighborIndex]) {
                continue;
            }

            costs[neighborIndex] = candidateCost;
            parents[neighborIndex] = node.index;
            open.push({ neighborIndex, candidateCost, ManhattanDistance(neighbor, goal), order++ });
        }
    }

    return false;
}
