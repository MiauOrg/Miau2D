#include "TiledMap.h"

#include <raylib.h>
#include <raymath.h>

#include "Character.hpp"

namespace {

void ClampCamera(Camera2D& camera, const Rectangle& bounds) {
    const float halfWidth = GetScreenWidth() * 0.5f / camera.zoom;
    const float halfHeight = GetScreenHeight() * 0.5f / camera.zoom;

    if (bounds.width > halfWidth * 2.0f) {
        camera.target.x = Clamp(camera.target.x, bounds.x + halfWidth, bounds.x + bounds.width - halfWidth);
    } else {
        camera.target.x = bounds.x + bounds.width * 0.5f;
    }

    if (bounds.height > halfHeight * 2.0f) {
        camera.target.y = Clamp(camera.target.y, bounds.y + halfHeight, bounds.y + bounds.height - halfHeight);
    } else {
        camera.target.y = bounds.y + bounds.height * 0.5f;
    }
}
}

const float VirtualResolutionWidth = 640.0f;
const float VirtualResolutionHeight = 352.0f;

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Hesan2D");

    if (!IsWindowReady()) {
        return 1;
    }

    SetTargetFPS(60);

    RenderTexture2D viewportTarget = LoadRenderTexture((int)VirtualResolutionWidth, (int)VirtualResolutionHeight);
    SetTextureFilter(viewportTarget.texture, TEXTURE_FILTER_POINT);

    TiledMap map;
    map.Load("tiled/Overworld01.tmx");

    const Rectangle bounds = map.Bounds();

    Character player({ 400.0f, 225.0f });
    Texture2D texture = LoadTexture("assets/characters/Character.png");

    Camera2D camera{};
    camera.zoom = 1.0f;
    camera.target = player.position - Vector2 { VirtualResolutionWidth / 2.0f, VirtualResolutionHeight / 2.0f };


    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        const float speed = 600.0f / camera.zoom;

        Vector2 dir = { 0.0f, 0.0f };
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dir.x -= 1.0f;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dir.x += 1.0f;
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dir.y -= 1.0f;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  dir.y += 1.0f;

        dir = Vector2Normalize(dir);

        camera.target = Vector2Lerp(camera.target, player.position - Vector2 { VirtualResolutionWidth / 2.0f, VirtualResolutionHeight / 2.0f }, dt * 5.0f);

        if (IsKeyDown(KEY_EQUAL) || IsKeyDown(KEY_KP_ADD)) camera.zoom += 2.0f * dt;
        if (IsKeyDown(KEY_MINUS) || IsKeyDown(KEY_KP_SUBTRACT)) camera.zoom -= 2.0f * dt;
        camera.zoom = Clamp(camera.zoom, 0.25f, 4.0f);

        player.Move(dir);
        player.Update(dt);
        
        // To eliminate sub pixel jitter
        Camera2D renderCamera = camera;
        renderCamera.target.x = floorf(camera.target.x);
        renderCamera.target.y = floorf(camera.target.y);

        // Virtual Viewport
        BeginTextureMode(viewportTarget);
            ClearBackground({16, 18, 27, 255});
            BeginMode2D(renderCamera);
                map.Draw();
                player.Draw(texture);
            EndMode2D();
        EndTextureMode();

        // Aspect Ratio
        const float screenWidth = (float)GetScreenWidth();
        const float screenHeight = (float)GetScreenHeight();
        const float scale = fminf(screenWidth / VirtualResolutionWidth, screenHeight / VirtualResolutionHeight);

        Rectangle srcRec = { 0.0f, 0.0f, VirtualResolutionWidth, -VirtualResolutionHeight };
        Rectangle destRec = {
            (screenWidth - (VirtualResolutionWidth * scale)) * 0.5f,
            (screenHeight - (VirtualResolutionHeight * scale)) * 0.5f,
            VirtualResolutionWidth * scale,
            VirtualResolutionHeight * scale
        };

        // GUI
        BeginDrawing();
            ClearBackground(BLACK);

            // Low res Viewport to screen
            DrawTexturePro(viewportTarget.texture, srcRec, destRec, {0, 0}, 0.0f, WHITE);
            DrawText("AAAAAAAAA", 20, 20, 24, RAYWHITE);
            DrawFPS((int)screenWidth - 100, 20);
        EndDrawing();
    }

    UnloadTexture(texture);
    UnloadRenderTexture(viewportTarget);
    map.Unload();
    CloseWindow();

    return 0;
}