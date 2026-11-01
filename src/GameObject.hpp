#pragma once

#include "Transform.hpp"

#include <raylib.h>

#include <cmath>

class Scene;

class GameObject {
public:
    virtual ~GameObject() = default;

    virtual void Update(Scene& scene, float deltaTime) {
        (void)scene;
        (void)deltaTime;
    }
    virtual void Draw() const {}
    virtual void DrawDebug() const {}

    // Whether this object should be collected for the y-sorted draw pass.
    [[nodiscard]] virtual bool IsVisible() const { return false; }

    // Objects are sorted by their ground point, not by sprite bounds.
    [[nodiscard]] virtual float SortY() const { return transform.position.y; }

    hesan::Transform transform;
    bool active = true;
};

class SpriteObject : public GameObject {
public:
    Texture2D texture{};
    Rectangle frame{ 0.0f, 0.0f, 0.0f, 0.0f };

    // Nudge the sort point if the art's base does not sit exactly on the anchor.
    float sortOffset = 0.0f;

    void Draw() const override {
        if (texture.id == 0 || frame.width <= 0.0f || frame.height <= 0.0f) {
            return;
        }

        // The anchor is the bottom-center of the sprite; draw upward from it.
        const Vector2 topLeft {
            std::floor(transform.position.x - frame.width * 0.5f),
            std::floor(transform.position.y - frame.height)
        };

        DrawTextureRec(texture, frame, topLeft, WHITE);
    }

    [[nodiscard]] bool IsVisible() const override { return texture.id != 0; }

    [[nodiscard]] float SortY() const override {
        return transform.position.y + sortOffset;
    }
};
