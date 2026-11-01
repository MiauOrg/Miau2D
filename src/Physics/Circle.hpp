#pragma once

#include <raylib.h>

struct Circle {
    Vector2 center{ 0.0f, 0.0f };
    float radius = 0.0f;
};

struct CircleContact {
    Vector2 normal{ 0.0f, 0.0f };
    float penetration = 0.0f;
};

class AABB;

bool CheckCircleCollision(const Circle& a, const Circle& b);
bool GetCircleContact(const Circle& a, const Circle& b, CircleContact& outContact);
bool CheckCircleAABBCollision(const Circle& circle, const AABB& box);
bool GetCircleAABBContact(const Circle& circle, const AABB& box, CircleContact& outContact);
bool CheckAABBCircleCollision(const AABB& box, const Circle& circle);
bool GetAABBCircleContact(const AABB& box, const Circle& circle, CircleContact& outContact);
