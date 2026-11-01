#include "AppConfig.hpp"
#include "Player.hpp"
#include "Scene.hpp"

#include <raylib.h>

#include <cmath>

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(WindowWidth, WindowHeight, "Hesan2D");

    if (!IsWindowReady()) {
        return 1;
    }

    SetTargetFPS(60);

    RenderTexture2D viewportTarget = LoadRenderTexture(
        static_cast<int>(VirtualWidth),
        static_cast<int>(VirtualHeight)
    );
    SetTextureFilter(viewportTarget.texture, TEXTURE_FILTER_POINT);

    Scene scene;
    if (!scene.Load()) {
        UnloadRenderTexture(viewportTarget);
        CloseWindow();
        return 1;
    }

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();

        if (IsKeyPressed(KEY_F1)) {
            scene.ToggleDebug();
        }

        scene.Update(dt);

        // Virtual viewport
        BeginTextureMode(viewportTarget);
            ClearBackground({ 16, 18, 27, 255 });
            scene.Draw();
        EndTextureMode();

        // Aspect ratio
        const float screenWidth = static_cast<float>(GetScreenWidth());
        const float screenHeight = static_cast<float>(GetScreenHeight());
        const float scale = fminf(
            screenWidth / VirtualWidth,
            screenHeight / VirtualHeight
        );

        const Rectangle srcRec = { 0.0f, 0.0f, VirtualWidth, -VirtualHeight };
        const Rectangle destRec = {
            (screenWidth - (VirtualWidth * scale)) * 0.5f,
            (screenHeight - (VirtualHeight * scale)) * 0.5f,
            VirtualWidth * scale,
            VirtualHeight * scale
        };

        const Player* player = scene.GetPlayer();

        BeginDrawing();
            ClearBackground(BLACK);

            DrawTexturePro(viewportTarget.texture, srcRec, destRec, { 0.0f, 0.0f }, 0.0f, WHITE);

            if (player != nullptr) {
                DrawText(
                    TextFormat("Movement: %s (F2 to toggle)",
                               player->GetMovementMode() == MovementMode::Grid ? "Grid" : "Free"),
                    20, 20, 16, SKYBLUE);
            }

            if (scene.IsDebugVisible()) {
                DrawText("Physics Debug: ON (F1 to toggle)", 20, 44, 16, LIME);

                if (player != nullptr) {
                    const Vector2 cell = player->GetCell();
                    DrawText(
                        TextFormat("Static Colliders: %d | Cell: %.0f, %.0f",
                                   static_cast<int>(scene.GetPhysics().GetColliderCount()),
                                   cell.x, cell.y),
                        20, 64, 16, RAYWHITE);
                }
            } else {
                DrawText("Press F1 to toggle Physics Debug", 20, 44, 16, GRAY);
            }

            DrawFPS(static_cast<int>(screenWidth) - 100, 20);
        EndDrawing();
    }

    scene.Unload();
    UnloadRenderTexture(viewportTarget);
    CloseWindow();

    return 0;
}
