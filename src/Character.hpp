#pragma once

#include "raylib.h"

class Character {
public:
    Vector2 position { 0.0f, 0.0f };
    Vector2 textureOffset { 0.0f, 0.0f };
    float speed = 100.0f;
    Vector2 velocity { 0.0f, 0.0f };
    Rectangle rect = {
        0.0f,   // x
        0.0f,   // y
        24.0f,  // width
        24.0f,  // height
    };

    void Move(Vector2 direction);
    void Update(float deltaTime);

    void Draw(Texture2D texture) const {
        DrawCircleV(position, 8.0f, RED);

        DrawTextureRec(texture, rect, position, WHITE);
    };
};