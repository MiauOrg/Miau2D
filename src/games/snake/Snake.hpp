#pragma once

#include "Game.hpp"
#include "SpriteAnimation.hpp"
#include "Viewport.hpp"

#include <vector>

class Snake final : public Game {
public:
    [[nodiscard]] GameConfig Config() const override;
    bool Load() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Unload() override;

private:
    struct Cell {
        int x = 0;
        int y = 0;

        bool operator==(const Cell&) const = default;
    };

    enum class Direction {
        Up,
        Down,
        Left,
        Right
    };

    static constexpr int VIEW_WIDTH = 640;
    static constexpr int VIEW_HEIGHT = 400;
    static constexpr int MAP_WIDTH = 112;
    static constexpr int MAP_HEIGHT = 112;
    static constexpr int MAP_CELL_PIXELS = 4;
    static constexpr int GRID_COLUMNS = MAP_WIDTH / MAP_CELL_PIXELS;
    static constexpr int GRID_ROWS = MAP_HEIGHT / MAP_CELL_PIXELS;
    static constexpr int CELL_SIZE = 12;
    static constexpr int BOARD_X = (VIEW_WIDTH - GRID_COLUMNS * CELL_SIZE) / 2;
    static constexpr int BOARD_Y = 40;
    static constexpr float STEP_INTERVAL = 0.12f;

    void Reset();
    void ReadInput();
    void Step();
    void SpawnFood();
    [[nodiscard]] bool Contains(Cell cell) const;
    [[nodiscard]] bool IsCellWalkable(Cell cell) const;
    [[nodiscard]] static bool IsOpposite(Direction first, Direction second);
    [[nodiscard]] static Cell Offset(Direction direction);
    void DrawSegment(Cell cell, Rectangle source, float rotation) const;

    Viewport viewport_;
    Texture2D mapTexture_{};
    Texture2D snakeTexture_{};
    Texture2D appleTexture_{};
    SpriteAnimation appleAnimation_;
    std::vector<unsigned char> walkable_;
    std::vector<Cell> body_;
    Cell food_{};
    Direction direction_ = Direction::Right;
    Direction queuedDirection_ = Direction::Right;
    float stepTimer_ = 0.0f;
    int score_ = 0;
    bool paused_ = false;
    bool gameOver_ = false;
    bool won_ = false;
};
