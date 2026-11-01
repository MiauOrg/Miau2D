#pragma once

#include <raylib.h>

// raylib already defines a 3D `Transform`, so the engine's 2D one lives in a
// namespace instead of taking a `2D` suffix.
namespace hesan {

struct Transform {
    Vector2 position{ 0.0f, 0.0f };
    float rotation = 0.0f;
    Vector2 scale{ 1.0f, 1.0f };
};

}  // namespace hesan
