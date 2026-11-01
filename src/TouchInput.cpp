#include "TouchInput.hpp"

bool TouchInput::ReadWorldTarget(Rectangle viewport, Vector2 virtualResolution,
                                 Camera2D camera, Vector2& outWorldTarget) const {
    bool pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    Vector2 screenPosition = GetMousePosition();

    if (GetTouchPointCount() > 0 && IsGestureDetected(GESTURE_TAP)) {
        screenPosition = GetTouchPosition(0);
        pressed = true;
    }

    if (!pressed || viewport.width <= 0.0f || viewport.height <= 0.0f ||
        virtualResolution.x <= 0.0f || virtualResolution.y <= 0.0f ||
        !CheckCollisionPointRec(screenPosition, viewport)) {
        return false;
    }

    const Vector2 viewportPosition{
        (screenPosition.x - viewport.x) * virtualResolution.x / viewport.width,
        (screenPosition.y - viewport.y) * virtualResolution.y / viewport.height
    };
    outWorldTarget = GetScreenToWorld2D(viewportPosition, camera);
    return true;
}
