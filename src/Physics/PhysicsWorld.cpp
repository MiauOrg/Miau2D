#include "Physics/PhysicsWorld.hpp"

#include <algorithm>
#include <cmath>

namespace {

Vector2 MoveAABBAgainstCircles(const AABB& box, Vector2 displacement,
                              const std::vector<Circle>& circles) {
    AABB current = box;
    const float distance = std::sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
    const float minExtent = std::min(box.size.x, box.size.y);
    const float maxStep = std::max(1.0f, minExtent * 0.5f);
    const int stepCount = std::clamp(static_cast<int>(std::ceil(distance / maxStep)), 1, 64);
    const Vector2 step = { displacement.x / stepCount, displacement.y / stepCount };

    constexpr int maxSlideIterations = 4;
    for (int stepIndex = 0; stepIndex < stepCount; ++stepIndex) {
        Vector2 remaining = step;

        for (int iteration = 0; iteration < maxSlideIterations; ++iteration) {
            AABB candidate = current;
            candidate.position.x += remaining.x;
            candidate.position.y += remaining.y;
            bool collided = false;

            for (const Circle& collider : circles) {
                CircleContact contact{};
                if (!GetAABBCircleContact(candidate, collider, contact)) {
                    continue;
                }

                candidate.position.x += contact.normal.x * (contact.penetration + 0.001f);
                candidate.position.y += contact.normal.y * (contact.penetration + 0.001f);

                const float inwardMovement = remaining.x * contact.normal.x + remaining.y * contact.normal.y;
                if (inwardMovement < 0.0f) {
                    remaining.x -= contact.normal.x * inwardMovement;
                    remaining.y -= contact.normal.y * inwardMovement;
                }

                collided = true;
            }

            current = candidate;
            if (!collided || remaining.x * remaining.x + remaining.y * remaining.y < 1e-8f) {
                break;
            }
        }
    }

    return { current.position.x - box.position.x, current.position.y - box.position.y };
}

}

void PhysicsWorld::AddStaticCollider(const AABB& collider) {
    staticColliders_.push_back(collider);
}

void PhysicsWorld::AddStaticCollider(const Rectangle& rect) {
    staticColliders_.emplace_back(rect);
}

void PhysicsWorld::AddStaticColliders(const std::vector<AABB>& colliders) {
    staticColliders_.insert(staticColliders_.end(), colliders.begin(), colliders.end());
}

void PhysicsWorld::ClearStaticColliders() {
    staticColliders_.clear();
}

void PhysicsWorld::AddStaticCircleCollider(const Circle& collider) {
    staticCircleColliders_.push_back(collider);
}

void PhysicsWorld::AddStaticCircleColliders(const std::vector<Circle>& colliders) {
    staticCircleColliders_.insert(staticCircleColliders_.end(), colliders.begin(), colliders.end());
}

void PhysicsWorld::ClearStaticCircleColliders() {
    staticCircleColliders_.clear();
}

void PhysicsWorld::SetWorldBounds(const Rectangle& bounds) {
    worldBounds_ = AABB(bounds);
}

void PhysicsWorld::ClearWorldBounds() {
    worldBounds_.reset();
}

bool PhysicsWorld::CheckCollision(const AABB& box) const {
    for (const auto& collider : staticColliders_) {
        if (box.Intersects(collider)) {
            return true;
        }
    }

    for (const auto& collider : staticCircleColliders_) {
        if (CheckAABBCircleCollision(box, collider)) {
            return true;
        }
    }

    return false;
}

std::vector<AABB> PhysicsWorld::Query(const AABB& area) const {
    std::vector<AABB> results;
    for (const auto& collider : staticColliders_) {
        if (area.Intersects(collider)) {
            results.push_back(collider);
        }
    }
    return results;
}

bool PhysicsWorld::CheckCollision(const Circle& circle) const {
    for (const auto& collider : staticCircleColliders_) {
        if (CheckCircleCollision(circle, collider)) {
            return true;
        }
    }

    for (const auto& collider : staticColliders_) {
        if (CheckCircleAABBCollision(circle, collider)) {
            return true;
        }
    }

    return false;
}

bool PhysicsWorld::GetCircleContact(const Circle& circle, CircleContact& outContact) const {
    outContact = CircleContact{};
    for (const auto& collider : staticCircleColliders_) {
        if (::GetCircleContact(circle, collider, outContact)) {
            return true;
        }
    }

    for (const auto& collider : staticColliders_) {
        if (GetCircleAABBContact(circle, collider, outContact)) {
            return true;
        }
    }

    return false;
}

std::vector<Circle> PhysicsWorld::Query(const Circle& area) const {
    std::vector<Circle> results;
    for (const auto& collider : staticCircleColliders_) {
        if (CheckCircleCollision(area, collider)) {
            results.push_back(collider);
        }
    }
    return results;
}

Vector2 PhysicsWorld::ResolveOverlap(const Circle& circle) const {
    if (!std::isfinite(circle.center.x) || !std::isfinite(circle.center.y) ||
        !std::isfinite(circle.radius) || circle.radius <= 0.0f) {
        return { 0.0f, 0.0f };
    }

    Circle current = circle;
    Vector2 totalDisplacement{ 0.0f, 0.0f };

    constexpr int maxIterations = 4;
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        bool resolvedAny = false;

        for (const Circle& collider : staticCircleColliders_) {
            CircleContact contact{};
            if (!::GetCircleContact(current, collider, contact)) {
                continue;
            }

            const Vector2 correction = {
                contact.normal.x * (contact.penetration + 0.001f),
                contact.normal.y * (contact.penetration + 0.001f)
            };
            current.center.x += correction.x;
            current.center.y += correction.y;
            totalDisplacement.x += correction.x;
            totalDisplacement.y += correction.y;
            resolvedAny = true;
        }

        for (const AABB& collider : staticColliders_) {
            CircleContact contact{};
            if (!GetCircleAABBContact(current, collider, contact)) {
                continue;
            }

            const Vector2 correction = {
                contact.normal.x * (contact.penetration + 0.001f),
                contact.normal.y * (contact.penetration + 0.001f)
            };
            current.center.x += correction.x;
            current.center.y += correction.y;
            totalDisplacement.x += correction.x;
            totalDisplacement.y += correction.y;
            resolvedAny = true;
        }

        if (!resolvedAny) {
            break;
        }
    }

    return totalDisplacement;
}

Vector2 PhysicsWorld::MoveAndSlide(const Circle& circle, Vector2 displacement) const {
    if (!std::isfinite(circle.center.x) || !std::isfinite(circle.center.y) ||
        !std::isfinite(circle.radius) || circle.radius <= 0.0f) {
        return { 0.0f, 0.0f };
    }

    Circle current = circle;
    const Vector2 initialPush = ResolveOverlap(current);
    current.center.x += initialPush.x;
    current.center.y += initialPush.y;
    const float distance = std::sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
    const float maxStep = std::max(1.0f, circle.radius * 0.5f);
    const int stepCount = std::clamp(static_cast<int>(std::ceil(distance / maxStep)), 1, 64);
    const Vector2 step = { displacement.x / stepCount, displacement.y / stepCount };

    constexpr int maxSlideIterations = 4;
    for (int stepIndex = 0; stepIndex < stepCount; ++stepIndex) {
        Vector2 remaining = step;

        for (int iteration = 0; iteration < maxSlideIterations; ++iteration) {
            Circle candidate = current;
            candidate.center.x += remaining.x;
            candidate.center.y += remaining.y;
            bool collided = false;

            for (const auto& collider : staticCircleColliders_) {
                CircleContact contact{};
                if (!::GetCircleContact(candidate, collider, contact)) {
                    continue;
                }

                candidate.center.x += contact.normal.x * (contact.penetration + 0.001f);
                candidate.center.y += contact.normal.y * (contact.penetration + 0.001f);

                const float inwardMovement = remaining.x * contact.normal.x + remaining.y * contact.normal.y;
                if (inwardMovement < 0.0f) {
                    remaining.x -= contact.normal.x * inwardMovement;
                    remaining.y -= contact.normal.y * inwardMovement;
                }

                collided = true;
            }

            for (const auto& collider : staticColliders_) {
                CircleContact contact{};
                if (!GetCircleAABBContact(candidate, collider, contact)) {
                    continue;
                }

                candidate.center.x += contact.normal.x * (contact.penetration + 0.001f);
                candidate.center.y += contact.normal.y * (contact.penetration + 0.001f);

                const float inwardMovement = remaining.x * contact.normal.x + remaining.y * contact.normal.y;
                if (inwardMovement < 0.0f) {
                    remaining.x -= contact.normal.x * inwardMovement;
                    remaining.y -= contact.normal.y * inwardMovement;
                }

                collided = true;
            }

            current = candidate;
            if (!collided || remaining.x * remaining.x + remaining.y * remaining.y < 1e-8f) {
                break;
            }
        }

        if (worldBounds_.has_value()) {
            const float minX = worldBounds_->Left() + circle.radius;
            const float maxX = worldBounds_->Right() - circle.radius;
            const float minY = worldBounds_->Top() + circle.radius;
            const float maxY = worldBounds_->Bottom() - circle.radius;

            if (minX <= maxX) current.center.x = std::clamp(current.center.x, minX, maxX);
            if (minY <= maxY) current.center.y = std::clamp(current.center.y, minY, maxY);
        }
    }

    return {
        current.center.x - circle.center.x,
        current.center.y - circle.center.y
    };
}

bool PhysicsWorld::Raycast(Vector2 origin, Vector2 direction, float maxDistance, RaycastHit& outHit) const {
    bool hitAny = false;
    float closestTime = maxDistance;
    RaycastHit closestHit{};

    for (const auto& collider : staticColliders_) {
        RaycastHit hit;
        if (collider.Raycast(origin, direction, maxDistance, hit)) {
            if (hit.time < closestTime) {
                closestTime = hit.time;
                closestHit = hit;
                hitAny = true;
            }
        }
    }

    if (hitAny) {
        outHit = closestHit;
    }
    return hitAny;
}

Vector2 PhysicsWorld::ResolveOverlap(const AABB& box) const {
    AABB current = box;
    Vector2 totalDisplacement{ 0.0f, 0.0f };

    constexpr int maxIterations = 4;
    for (int iter = 0; iter < maxIterations; ++iter) {
        bool resolvedAny = false;
        for (const auto& collider : staticColliders_) {
            Vector2 mtv{ 0.0f, 0.0f };
            if (current.ResolveCollision(collider, mtv)) {
                current.position.x += mtv.x;
                current.position.y += mtv.y;
                totalDisplacement.x += mtv.x;
                totalDisplacement.y += mtv.y;
                resolvedAny = true;
            }
        }

        for (const auto& collider : staticCircleColliders_) {
            CircleContact contact{};
            if (GetAABBCircleContact(current, collider, contact)) {
                const Vector2 correction = {
                    contact.normal.x * (contact.penetration + 0.001f),
                    contact.normal.y * (contact.penetration + 0.001f)
                };
                current.position.x += correction.x;
                current.position.y += correction.y;
                totalDisplacement.x += correction.x;
                totalDisplacement.y += correction.y;
                resolvedAny = true;
            }
        }

        if (!resolvedAny) break;
    }

    return totalDisplacement;
}

Vector2 PhysicsWorld::MoveAndSlide(const AABB& box, Vector2 displacement) const {
    AABB current = box;
    const Vector2 initialPush = ResolveOverlap(current);
    current.position.x += initialPush.x;
    current.position.y += initialPush.y;
    const Vector2 startAfterPush = current.position;

    if (displacement.x > 0.0f) {
        float newX = current.position.x + displacement.x;
        for (const auto& collider : staticColliders_) {
            if (current.Top() < collider.Bottom() && current.Bottom() > collider.Top()) {
                if (collider.Left() >= current.Right() - 0.001f && collider.Left() < newX + current.size.x) {
                    newX = std::min(newX, collider.Left() - current.size.x);
                }
            }
        }
        current.position.x = newX;
    } else if (displacement.x < 0.0f) {
        float newX = current.position.x + displacement.x;
        for (const auto& collider : staticColliders_) {
            if (current.Top() < collider.Bottom() && current.Bottom() > collider.Top()) {
                if (collider.Right() <= current.Left() + 0.001f && collider.Right() > newX) {
                    newX = std::max(newX, collider.Right());
                }
            }
        }
        current.position.x = newX;
    }

    if (displacement.y > 0.0f) {
        float newY = current.position.y + displacement.y;
        for (const auto& collider : staticColliders_) {
            if (current.Left() < collider.Right() && current.Right() > collider.Left()) {
                if (collider.Top() >= current.Bottom() - 0.001f && collider.Top() < newY + current.size.y) {
                    newY = std::min(newY, collider.Top() - current.size.y);
                }
            }
        }
        current.position.y = newY;
    } else if (displacement.y < 0.0f) {
        float newY = current.position.y + displacement.y;
        for (const auto& collider : staticColliders_) {
            if (current.Left() < collider.Right() && current.Right() > collider.Left()) {
                if (collider.Bottom() <= current.Top() + 0.001f && collider.Bottom() > newY) {
                    newY = std::max(newY, collider.Bottom());
                }
            }
        }
        current.position.y = newY;
    }

    const Vector2 rectangleMovement = {
        current.position.x - startAfterPush.x,
        current.position.y - startAfterPush.y
    };
    const Vector2 circleMovement = MoveAABBAgainstCircles(
        AABB(startAfterPush, box.size), rectangleMovement, staticCircleColliders_);
    current.position = {
        startAfterPush.x + circleMovement.x,
        startAfterPush.y + circleMovement.y
    };

    const Vector2 finalCorrection = ResolveOverlap(current);
    current.position.x += finalCorrection.x;
    current.position.y += finalCorrection.y;

    if (worldBounds_.has_value()) {
        current.position.x = std::clamp(
            current.position.x,
            worldBounds_->Left(),
            worldBounds_->Right() - current.size.x
        );
        current.position.y = std::clamp(
            current.position.y,
            worldBounds_->Top(),
            worldBounds_->Bottom() - current.size.y
        );
    }

    return {
        current.position.x - box.position.x,
        current.position.y - box.position.y
    };
}

void PhysicsWorld::DrawDebug(Color fillColor, Color outlineColor, float thickness) const {
    for (const auto& collider : staticColliders_) {
        collider.Draw(fillColor);
        collider.DrawLines(outlineColor, thickness);
    }

    for (std::size_t i = 0; i < staticCircleColliders_.size(); ++i) {
        bool overlapsAnotherCircle = false;
        for (std::size_t j = 0; j < staticCircleColliders_.size(); ++j) {
            if (i != j && CheckCircleCollision(staticCircleColliders_[i], staticCircleColliders_[j])) {
                overlapsAnotherCircle = true;
                break;
            }
        }

        const auto& circle = staticCircleColliders_[i];
        bool overlapsBox = false;
        for (const AABB& box : staticColliders_) {
            if (CheckCircleAABBCollision(circle, box)) {
                overlapsBox = true;
                break;
            }
        }
        DrawCircleLinesV(circle.center, circle.radius,
                         overlapsAnotherCircle || overlapsBox ? RED : SKYBLUE);
    }

    if (worldBounds_.has_value()) {
        worldBounds_->DrawLines(BLUE, thickness + 1.0f);
    }
}
