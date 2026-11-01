#pragma once

#include "SpriteAnimation.hpp"
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

    [[nodiscard]] virtual float SortY() const { return transform.position.y; }

    hesan::Transform transform;
    bool active = true;
};

class SpriteObject : public GameObject {
public:
    Texture2D texture{};
    Rectangle frame{ 0.0f, 0.0f, 0.0f, 0.0f };
    SpriteAnimation animation;
    Color tint = WHITE;
    float sortOffset = 0.0f;

    void Update(Scene& scene, float deltaTime) override {
        GameObject::Update(scene, deltaTime);
        animation.Update(deltaTime);
        if (animation.HasFrames()) {
            frame = animation.GetSourceRectangle();
        }
    }

    void Draw() const override {
        if (texture.id == 0 || frame.width <= 0.0f || frame.height <= 0.0f) {
            return;
        }

        const Vector2 topLeft {
            std::floor(transform.position.x - frame.width * 0.5f),
            std::floor(transform.position.y - frame.height)
        };

        DrawTextureRec(texture, frame, topLeft, tint);
    }

    [[nodiscard]] bool IsVisible() const override { return texture.id != 0; }

    [[nodiscard]] float SortY() const override {
        return transform.position.y + sortOffset;
    }
};
