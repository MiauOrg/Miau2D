#include "GridPathFollower.hpp"

#include "Actor.hpp"
#include "Movement.hpp"

#include <cmath>
#include <utility>

void GridPathFollower::SetPath(std::vector<GridCell> path) {
    path_ = std::move(path);
    nextCell_ = 0;
}

void GridPathFollower::Clear() {
    path_.clear();
    nextCell_ = 0;
}

void GridPathFollower::Update(Actor& actor) {
    if (actor.GetMovementMode() != MovementMode::Grid) {
        Movement::Move(actor, { 0.0f, 0.0f });
        return;
    }

    if (actor.IsGridMoving()) {
        Movement::Move(actor, { 0.0f, 0.0f });
        return;
    }

    const Vector2 cell = actor.GetCell();
    const GridCell current{ static_cast<int>(std::round(cell.x)), static_cast<int>(std::round(cell.y)) };
    while (nextCell_ < path_.size() && path_[nextCell_] == current) {
        ++nextCell_;
    }

    if (nextCell_ >= path_.size()) {
        Movement::Move(actor, { 0.0f, 0.0f });
        return;
    }

    const GridCell target = path_[nextCell_];
    const int dx = target.x - current.x;
    const int dy = target.y - current.y;
    if (std::abs(dx) + std::abs(dy) != 1) {
        Clear();
        Movement::Move(actor, { 0.0f, 0.0f });
        return;
    }

    Movement::Move(actor, { static_cast<float>(dx), static_cast<float>(dy) });
}

bool GridPathFollower::IsActive() const {
    return nextCell_ < path_.size();
}
