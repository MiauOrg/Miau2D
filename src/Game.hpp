#pragma once

#include "AppConfig.hpp"

#include <memory>

struct GameConfig {
    int windowWidth = WindowWidth;
    int windowHeight = WindowHeight;
    const char* title = "Hesan2D";
    int targetFps = TargetFps;
};

class Game {
public:
    virtual ~Game() = default;

    [[nodiscard]] virtual GameConfig Config() const { return {}; }
    virtual bool Load() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void Unload() = 0;
};

[[nodiscard]] std::unique_ptr<Game> CreateGame();
