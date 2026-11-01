#include "WanderingBrain.hpp"

#include "Actor.hpp"
#include "Physics/GridPathfinder.hpp"
#include "Movement.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

WanderingBrain::WanderingBrain(unsigned int randomSeed)
    : random_(randomSeed) {
}

void WanderingBrain::Update(Actor& actor, const GridPathfinder& pathfinder, float deltaTime) {
    follower_.Update(actor);
    if (follower_.IsActive() || actor.IsGridMoving()) {
        return;
    }

    waitRemaining_ = std::max(0.0f, waitRemaining_ - deltaTime);
    if (waitRemaining_ > 0.0f) {
        Movement::Move(actor, { 0.0f, 0.0f });
        return;
    }

    if (ChooseDestination(actor, pathfinder)) {
        waitRemaining_ = 0.6f + static_cast<float>(random_() % 120) / 100.0f;
        follower_.Update(actor);
    } else {
        waitRemaining_ = 0.5f;
        Movement::Move(actor, { 0.0f, 0.0f });
    }
}

bool WanderingBrain::ChooseDestination(Actor& actor, const GridPathfinder& pathfinder) {
    if (pathfinder.GetColumnCount() <= 0 || pathfinder.GetRowCount() <= 0) {
        return false;
    }

    const Vector2 current = actor.GetCell();
    const GridCell start{ static_cast<int>(std::round(current.x)), static_cast<int>(std::round(current.y)) };
    const Vector2 actorSize = actor.collider.size;
    std::uniform_int_distribution<int> xDistribution(0, pathfinder.GetColumnCount() - 1);
    std::uniform_int_distribution<int> yDistribution(0, pathfinder.GetRowCount() - 1);
    std::vector<GridCell> path;

    for (int attempt = 0; attempt < 64; ++attempt) {
        const GridCell goal{ xDistribution(random_), yDistribution(random_) };
        if (goal == start || !pathfinder.FindPath(start, goal, actorSize, path) || path.empty()) {
            continue;
        }

        follower_.SetPath(std::move(path));
        return true;
    }

    return false;
}
