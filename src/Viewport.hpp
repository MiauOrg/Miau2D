#pragma once

#include <raylib.h>

class Viewport {
public:
    Viewport() = default;
    ~Viewport();

    Viewport(const Viewport&) = delete;
    Viewport& operator=(const Viewport&) = delete;
    Viewport(Viewport&&) = delete;
    Viewport& operator=(Viewport&&) = delete;

    bool Load(int width, int height);
    void Unload();

    void Begin() const;
    void End() const;
    void Present() const;

    [[nodiscard]] int Width() const { return width_; }
    [[nodiscard]] int Height() const { return height_; }
    [[nodiscard]] Vector2 Size() const;
    [[nodiscard]] Rectangle ScreenRect() const;
    [[nodiscard]] Vector2 ToVirtual(Vector2 screenPosition) const;

private:
    RenderTexture2D target_{};
    int width_ = 0;
    int height_ = 0;
};
