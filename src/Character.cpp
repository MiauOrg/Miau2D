#include "Character.hpp"
#include <cmath>

void Character::Move(Vector2 direction)
{
    // Normalize so diagonal movement isn't faster.
    const float length = std::sqrt(
        direction.x * direction.x +
        direction.y * direction.y
    );

    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
    }

    // Store movement intent for Update().
    velocity = {
        direction.x * speed,
        direction.y * speed
    };
}

void Character::Update(float deltaTime)
{
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
}