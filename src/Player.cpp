#include "Player.hpp"

#include "Physics/GridPathfinder.hpp"
#include "Movement.hpp"
#include "Scene.hpp"
#include "Viewport.hpp"

#include <raylib.h>

#include <cmath>
#include <utility>
#include <vector>

void Player::Update(Scene& scene, float deltaTime) {
    if (IsKeyPressed(KEY_F2)) {
        const MovementMode nextMode = (mover.mode == MovementMode::Grid)
            ? MovementMode::Continuous
            : MovementMode::Grid;
        Movement::SetMovementMode(*this, nextMode, &scene.GetPhysics());
        touchPath_.Clear();
    }

    const Viewport& viewport = scene.GetViewport();
    Vector2 target{};
    if (touchInput_.ReadWorldTarget(
            viewport.ScreenRect(),
            viewport.Size(),
            scene.GetCamera(),
            target)) {
        touchPath_.Clear();
        if (mover.mode == MovementMode::Grid) {
            const Vector2 cell = GetCell();
            const GridCell start{ static_cast<int>(std::round(cell.x)), static_cast<int>(std::round(cell.y)) };
            std::vector<GridCell> path;
            if (scene.GetPathfinder().FindPath(start, scene.GetPathfinder().WorldToCell(target), collider.size, path)) {
                touchPath_.SetPath(std::move(path));
            }
        }
    }

    const Vector2 direction = input_.ReadMovement(mover.mode);
    if (direction.x != 0.0f || direction.y != 0.0f) {
        touchPath_.Clear();
        Movement::Move(*this, direction);
    } else if (touchPath_.IsActive()) {
        touchPath_.Update(*this);
    } else {
        Movement::Move(*this, { 0.0f, 0.0f });
    }

    Actor::Update(scene, deltaTime);
}
