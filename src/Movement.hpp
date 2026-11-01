#pragma once

#include "Actor.hpp"

class PhysicsWorld;

namespace Movement {

[[nodiscard]] Vector2 DominantDirection(Vector2 direction);
[[nodiscard]] bool IsZero(Vector2 value);
[[nodiscard]] bool SameDirection(Vector2 a, Vector2 b);

void SetMovementMode(Actor& actor, MovementMode mode, const PhysicsWorld* world = nullptr);
void SetTileSize(Actor& actor, Vector2 size);
void SetStepDuration(Actor& actor, float seconds);

// Moves the actor to the nearest free tile.
void SnapToGrid(Actor& actor, const PhysicsWorld* world = nullptr);

// Records the movement intent. Grid movement buffers a turn while stepping.
void Move(Actor& actor, Vector2 direction);

// Advances the actor according to its movement mode.
void Update(Actor& actor, float deltaTime, const PhysicsWorld* world = nullptr);

}  // namespace Movement
