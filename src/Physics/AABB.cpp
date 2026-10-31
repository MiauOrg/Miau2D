#include "Physics/AABB.hpp"

bool AABB::GetOverlap(const AABB& other, Vector2& outOverlap) const {
    if (!Intersects(other)) {
        outOverlap = { 0.0f, 0.0f };
        return false;
    }

    const float overlapX = std::min(Right(), other.Right()) - std::max(Left(), other.Left());
    const float overlapY = std::min(Bottom(), other.Bottom()) - std::max(Top(), other.Top());

    outOverlap = { overlapX, overlapY };
    return true;
}

bool AABB::ResolveCollision(const AABB& staticBox, Vector2& outDisplacement) const {
    Vector2 overlap{ 0.0f, 0.0f };
    if (!GetOverlap(staticBox, overlap)) {
        outDisplacement = { 0.0f, 0.0f };
        return false;
    }

    const Vector2 centerA = Center();
    const Vector2 centerB = staticBox.Center();

    if (overlap.x < overlap.y) {
        outDisplacement.x = (centerA.x < centerB.x) ? -overlap.x : overlap.x;
        outDisplacement.y = 0.0f;
    } else {
        outDisplacement.x = 0.0f;
        outDisplacement.y = (centerA.y < centerB.y) ? -overlap.y : overlap.y;
    }

    return true;
}

bool AABB::Raycast(Vector2 origin, Vector2 direction, float maxDistance, RaycastHit& outHit) const {
    outHit = RaycastHit{};

    const float lenSq = direction.x * direction.x + direction.y * direction.y;
    if (lenSq == 0.0f) {
        if (Contains(origin)) {
            outHit.hit = true;
            outHit.point = origin;
            return true;
        }
        return false;
    }

    float tNearX = Left() - origin.x;
    float tFarX  = Right() - origin.x;
    if (direction.x < 0.0f) std::swap(tNearX, tFarX);

    float tNearY = Top() - origin.y;
    float tFarY  = Bottom() - origin.y;
    if (direction.y < 0.0f) std::swap(tNearY, tFarY);

    if (std::abs(direction.x) > 1e-8f) {
        tNearX /= direction.x;
        tFarX  /= direction.x;
    } else {
        if (origin.x < Left() || origin.x > Right()) return false;
        tNearX = -std::numeric_limits<float>::infinity();
        tFarX  =  std::numeric_limits<float>::infinity();
    }

    if (std::abs(direction.y) > 1e-8f) {
        tNearY /= direction.y;
        tFarY  /= direction.y;
    } else {
        if (origin.y < Top() || origin.y > Bottom()) return false;
        tNearY = -std::numeric_limits<float>::infinity();
        tFarY  =  std::numeric_limits<float>::infinity();
    }

    const float tEnter = std::max(tNearX, tNearY);
    const float tExit  = std::min(tFarX, tFarY);

    if (tEnter > tExit || tExit < 0.0f || tEnter > maxDistance) {
        return false;
    }

    outHit.hit = true;
    outHit.time = std::max(0.0f, tEnter);
    outHit.distance = outHit.time;
    outHit.point = { origin.x + direction.x * outHit.time, origin.y + direction.y * outHit.time };

    if (tNearX > tNearY) {
        outHit.normal = (direction.x < 0.0f) ? Vector2{ 1.0f, 0.0f } : Vector2{ -1.0f, 0.0f };
    } else {
        outHit.normal = (direction.y < 0.0f) ? Vector2{ 0.0f, 1.0f } : Vector2{ 0.0f, -1.0f };
    }

    return true;
}

bool AABB::SweptAABB(Vector2 displacement, const AABB& target, RaycastHit& outHit) const {
    outHit = RaycastHit{};

    if (Intersects(target)) {
        outHit.hit = true;
        outHit.time = 0.0f;
        outHit.distance = 0.0f;
        outHit.point = Center();
        Vector2 disp{ 0.0f, 0.0f };
        ResolveCollision(target, disp);
        if (disp.x != 0.0f) outHit.normal = { (disp.x > 0.0f) ? 1.0f : -1.0f, 0.0f };
        else if (disp.y != 0.0f) outHit.normal = { 0.0f, (disp.y > 0.0f) ? 1.0f : -1.0f };
        return true;
    }

    if (displacement.x == 0.0f && displacement.y == 0.0f) {
        return false;
    }

    const AABB expanded(
        target.position.x - size.x,
        target.position.y - size.y,
        target.size.x + size.x,
        target.size.y + size.y
    );

    return expanded.Raycast(position, displacement, 1.0f, outHit);
}

void AABB::Draw(Color color) const {
    DrawRectangleRec(ToRectangle(), color);
}

void AABB::DrawLines(Color color, float thickness) const {
    if (thickness <= 1.0f) {
        DrawRectangleLines(
            static_cast<int>(position.x),
            static_cast<int>(position.y),
            static_cast<int>(size.x),
            static_cast<int>(size.y),
            color
        );
    } else {
        DrawRectangleLinesEx(ToRectangle(), thickness, color);
    }
}
