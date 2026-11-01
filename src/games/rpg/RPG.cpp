#include "RPG.hpp"

#include "AppConfig.hpp"
#include "Player.hpp"

#include <raylib.h>

GameConfig RPG::Config() const {
    return { WindowWidth, WindowHeight, "Hesan2D", TargetFps };
}

bool RPG::Load() {
    return scene_.Load();
}

void RPG::Update(float deltaTime) {
    if (IsKeyPressed(KEY_F1)) {
        scene_.ToggleDebug();
    }

    scene_.Update(deltaTime);
}

void RPG::Draw() {
    scene_.Draw();

    const Player* player = scene_.GetPlayer();
    if (player != nullptr) {
        DrawText(
            TextFormat("Movement: %s (F2 to toggle)",
                       player->GetMovementMode() == MovementMode::Grid ? "Grid" : "Free"),
            20, 20, 16, SKYBLUE);
    }

    DrawText("Click/tap to pathfind in Grid mode | NPC wanders", 20, 42, 16, LIGHTGRAY);

    if (scene_.IsDebugVisible()) {
        DrawText("Physics Debug: ON (F1 to toggle)", 20, 64, 16, LIME);

        if (player != nullptr) {
            const Vector2 cell = player->GetCell();
            DrawText(
                TextFormat("Static Colliders: %d | Cell: %.0f, %.0f",
                           static_cast<int>(scene_.GetPhysics().GetColliderCount()),
                           cell.x, cell.y),
                20, 84, 16, RAYWHITE);
        }
    } else {
        DrawText("Press F1 to toggle Physics Debug", 20, 64, 16, GRAY);
    }

    DrawFPS(GetScreenWidth() - 100, 20);
}

void RPG::Unload() {
    scene_.Unload();
}
