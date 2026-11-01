#pragma once

#include "Actor.hpp"

class PlayerInput {
public:
    [[nodiscard]] Vector2 ReadMovement(MovementMode mode);

private:
    bool preferHorizontal_ = true;
};
