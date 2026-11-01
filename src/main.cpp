#include "Game.hpp"

#include <raylib.h>

#include <memory>

int main() {
    {
        std::unique_ptr<Game> game = CreateGame();
        if (!game) {
            return 1;
        }

        const GameConfig config = game->Config();
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
        InitWindow(config.windowWidth, config.windowHeight, config.title);

        if (!IsWindowReady()) {
            return 1;
        }

        SetTargetFPS(config.targetFps);

        if (!game->Load()) {
            game->Unload();
            CloseWindow();
            return 1;
        }

        while (!WindowShouldClose()) {
            game->Update(GetFrameTime());

            BeginDrawing();
            ClearBackground(BLACK);
            game->Draw();
            EndDrawing();
        }

        game->Unload();
    }

    CloseWindow();
    return 0;
}
