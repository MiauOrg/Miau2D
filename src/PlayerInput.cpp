#include "PlayerInput.hpp"

#include <cmath>

Vector2 PlayerInput::ReadMovement(MovementMode mode) {
    Vector2 direction{ 0.0f, 0.0f };
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direction.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direction.y += 1.0f;

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) ||
        IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        preferHorizontal_ = false;
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT) ||
        IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        preferHorizontal_ = true;
    }

    if (mode == MovementMode::Grid && direction.x != 0.0f && direction.y != 0.0f) {
        if (preferHorizontal_) {
            direction.y = 0.0f;
        } else {
            direction.x = 0.0f;
        }
    }

    const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
    }
    return direction;
}
