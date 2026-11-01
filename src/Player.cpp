#include "Player.hpp"

#include "Movement.hpp"
#include "Scene.hpp"

#include <raylib.h>
#include <raymath.h>

void Player::Update(Scene& scene, float deltaTime) {
    if (IsKeyPressed(KEY_F2)) {
        const MovementMode nextMode = (mover.mode == MovementMode::Grid)
            ? MovementMode::Continuous
            : MovementMode::Grid;
        Movement::SetMovementMode(*this, nextMode, &scene.GetPhysics());
    }

    Vector2 direction = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  direction.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    direction.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  direction.y += 1.0f;

    // Grid movement is 4-directional. When two axes are held, use the one
    // that was pressed most recently so turning works while holding a key.
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) ||
        IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        preferHorizontal_ = false;
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT) ||
        IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        preferHorizontal_ = true;
    }
    if (mover.mode == MovementMode::Grid && direction.x != 0.0f && direction.y != 0.0f) {
        if (preferHorizontal_) {
            direction.y = 0.0f;
        } else {
            direction.x = 0.0f;
        }
    }

    Movement::Move(*this, Vector2Normalize(direction));
    Actor::Update(scene, deltaTime);
}
