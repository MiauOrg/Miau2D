#include "Physics/Circle.hpp"

#include "Physics/AABB.hpp"

#include <algorithm>
#include <cmath>

namespace {

bool IsValid(const Circle& circle) {
    return std::isfinite(circle.center.x) && std::isfinite(circle.center.y) &&
           std::isfinite(circle.radius) && circle.radius > 0.0f;
}

}

bool CheckCircleCollision(const Circle& a, const Circle& b) {
    if (!IsValid(a) || !IsValid(b)) {
        return false;
    }

    const float dx = a.center.x - b.center.x;
    const float dy = a.center.y - b.center.y;
    const float radiusSum = a.radius + b.radius;
    return dx * dx + dy * dy < radiusSum * radiusSum;
}

bool GetCircleContact(const Circle& a, const Circle& b, CircleContact& outContact) {
    outContact = CircleContact{};
    if (!CheckCircleCollision(a, b)) {
        return false;
    }

    const float dx = a.center.x - b.center.x;
    const float dy = a.center.y - b.center.y;
    const float distanceSquared = dx * dx + dy * dy;
    const float radiusSum = a.radius + b.radius;

    if (distanceSquared == 0.0f) {
        outContact.normal = { 1.0f, 0.0f };
        outContact.penetration = radiusSum;
        return true;
    }

    const float distance = std::sqrt(distanceSquared);
    outContact.normal = { dx / distance, dy / distance };
    outContact.penetration = radiusSum - distance;
    return true;
}

bool CheckCircleAABBCollision(const Circle& circle, const AABB& box) {
    if (!std::isfinite(circle.center.x) || !std::isfinite(circle.center.y) ||
        !std::isfinite(circle.radius) || circle.radius <= 0.0f ||
        !std::isfinite(box.position.x) || !std::isfinite(box.position.y) ||
        !std::isfinite(box.size.x) || !std::isfinite(box.size.y) ||
        box.size.x < 0.0f || box.size.y < 0.0f) {
        return false;
    }

    const float closestX = std::clamp(circle.center.x, box.Left(), box.Right());
    const float closestY = std::clamp(circle.center.y, box.Top(), box.Bottom());
    const float dx = circle.center.x - closestX;
    const float dy = circle.center.y - closestY;
    return dx * dx + dy * dy < circle.radius * circle.radius;
}

bool GetCircleAABBContact(const Circle& circle, const AABB& box, CircleContact& outContact) {
    outContact = CircleContact{};
    if (!CheckCircleAABBCollision(circle, box)) {
        return false;
    }

    const float closestX = std::clamp(circle.center.x, box.Left(), box.Right());
    const float closestY = std::clamp(circle.center.y, box.Top(), box.Bottom());
    const float dx = circle.center.x - closestX;
    const float dy = circle.center.y - closestY;
    const float distanceSquared = dx * dx + dy * dy;

    if (distanceSquared > 0.0f) {
        const float distance = std::sqrt(distanceSquared);
        outContact.normal = { dx / distance, dy / distance };
        outContact.penetration = circle.radius - distance;
        return true;
    }

    const float distances[] = {
        circle.center.x - box.Left(),
        box.Right() - circle.center.x,
        circle.center.y - box.Top(),
        box.Bottom() - circle.center.y
    };
    const Vector2 normals[] = {
        { -1.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, -1.0f },
        { 0.0f, 1.0f }
    };

    int nearestFace = 0;
    for (int index = 1; index < 4; ++index) {
        if (distances[index] < distances[nearestFace]) {
            nearestFace = index;
        }
    }

    outContact.normal = normals[nearestFace];
    outContact.penetration = distances[nearestFace] + circle.radius;
    return true;
}

bool CheckAABBCircleCollision(const AABB& box, const Circle& circle) {
    return CheckCircleAABBCollision(circle, box);
}

bool GetAABBCircleContact(const AABB& box, const Circle& circle, CircleContact& outContact) {
    if (!GetCircleAABBContact(circle, box, outContact)) {
        return false;
    }

    outContact.normal.x = -outContact.normal.x;
    outContact.normal.y = -outContact.normal.y;
    return true;
}
