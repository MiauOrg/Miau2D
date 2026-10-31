#pragma once

#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <limits>

struct RaycastHit {
    bool hit = false;
    float distance = 0.0f;
    float time = 0.0f;
    Vector2 point{ 0.0f, 0.0f };
    Vector2 normal{ 0.0f, 0.0f };
};

class AABB {
public:
    Vector2 position{ 0.0f, 0.0f };
    Vector2 size{ 0.0f, 0.0f };

    constexpr AABB() = default;
    constexpr AABB(Vector2 pos, Vector2 sz) : position(pos), size(sz) {}
    constexpr AABB(float x, float y, float width, float height) : position{ x, y }, size{ width, height } {}
    constexpr AABB(const Rectangle& rect) : position{ rect.x, rect.y }, size{ rect.width, rect.height } {}

    float Left()   const { return position.x; }
    float Right()  const { return position.x + size.x; }
    float Top()    const { return position.y; }
    float Bottom() const { return position.y + size.y; }

    Vector2 Center() const { return { position.x + size.x * 0.5f, position.y + size.y * 0.5f }; }
    Vector2 HalfSize() const { return { size.x * 0.5f, size.y * 0.5f }; }
    Vector2 Min() const { return position; }
    Vector2 Max() const { return { position.x + size.x, position.y + size.y }; }

    Rectangle ToRectangle() const { return { position.x, position.y, size.x, size.y }; }
    operator Rectangle() const { return ToRectangle(); }

    bool Contains(Vector2 point) const {
        return point.x >= Left() && point.x <= Right() &&
               point.y >= Top()  && point.y <= Bottom();
    }

    bool Contains(const AABB& other) const {
        return other.Left() >= Left() && other.Right() <= Right() &&
               other.Top() >= Top()   && other.Bottom() <= Bottom();
    }

    bool Intersects(const AABB& other) const {
        return
            Left() < other.Right() &&
            Right() > other.Left() &&
            Top() < other.Bottom() &&
            Bottom() > other.Top();
    }

    bool GetOverlap(const AABB& other, Vector2& outOverlap) const;
    bool ResolveCollision(const AABB& staticBox, Vector2& outDisplacement) const;

    bool Raycast(Vector2 origin, Vector2 direction, float maxDistance, RaycastHit& outHit) const;
    bool SweptAABB(Vector2 displacement, const AABB& target, RaycastHit& outHit) const;

    void Draw(Color color) const;
    void DrawLines(Color color, float thickness = 1.0f) const;
};
