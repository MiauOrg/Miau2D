#include "Viewport.hpp"

#include <cmath>

Viewport::~Viewport() {
    Unload();
}

bool Viewport::Load(int width, int height) {
    Unload();

    if (width <= 0 || height <= 0) {
        return false;
    }

    target_ = LoadRenderTexture(width, height);
    if (target_.id == 0) {
        return false;
    }

    SetTextureFilter(target_.texture, TEXTURE_FILTER_POINT);
    width_ = width;
    height_ = height;
    return true;
}

void Viewport::Unload() {
    if (target_.id != 0) {
        UnloadRenderTexture(target_);
        target_ = RenderTexture2D{};
    }

    width_ = 0;
    height_ = 0;
}

void Viewport::Begin() const {
    BeginTextureMode(target_);
}

void Viewport::End() const {
    EndTextureMode();
}

void Viewport::Present() const {
    if (target_.id == 0) {
        return;
    }

    const Rectangle destination = ScreenRect();
    const Rectangle source {
        0.0f,
        0.0f,
        static_cast<float>(width_),
        static_cast<float>(-height_)
    };

    DrawTexturePro(target_.texture, source, destination, { 0.0f, 0.0f }, 0.0f, WHITE);
}

Vector2 Viewport::Size() const {
    return {
        static_cast<float>(width_),
        static_cast<float>(height_)
    };
}

Rectangle Viewport::ScreenRect() const {
    if (width_ <= 0 || height_ <= 0) {
        return {};
    }

    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());
    const float virtualWidth = static_cast<float>(width_);
    const float virtualHeight = static_cast<float>(height_);
    const float scale = std::fmin(screenWidth / virtualWidth, screenHeight / virtualHeight);

    return {
        (screenWidth - virtualWidth * scale) * 0.5f,
        (screenHeight - virtualHeight * scale) * 0.5f,
        virtualWidth * scale,
        virtualHeight * scale
    };
}

Vector2 Viewport::ToVirtual(Vector2 screenPosition) const {
    const Rectangle rect = ScreenRect();
    if (rect.width <= 0.0f || rect.height <= 0.0f) {
        return {};
    }

    return {
        (screenPosition.x - rect.x) * static_cast<float>(width_) / rect.width,
        (screenPosition.y - rect.y) * static_cast<float>(height_) / rect.height
    };
}
