#pragma once

#include <raylib.h>

class TouchInput {
public:
    [[nodiscard]] bool ReadWorldTarget(Rectangle viewport, Vector2 virtualResolution,
                                       Camera2D camera, Vector2& outWorldTarget) const;
};
