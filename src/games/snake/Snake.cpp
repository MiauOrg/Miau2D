#include "Snake.hpp"

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

namespace {
constexpr Rectangle SNAKE_HEAD_FRAME{ 20.0f, 43.0f, 40.0f, 40.0f };
constexpr Rectangle SNAKE_BODY_FRAME{ 344.0f, 43.0f, 40.0f, 40.0f };
}

GameConfig Snake::Config() const {
    return { 960, 600, "Hesan2D - Snake", 60 };
}

bool Snake::Load() {
    if (!viewport_.Load(VIEW_WIDTH, VIEW_HEIGHT)) {
        return false;
    }
    SetRandomSeed(static_cast<unsigned int>(GetTime() * 1000000.0));

    Image mapImage = LoadImage("assets/snake_assets/BlobMap_16px.png");
    if (mapImage.data == nullptr || mapImage.width != MAP_WIDTH || mapImage.height != MAP_HEIGHT) {
        UnloadImage(mapImage);
        Unload();
        return false;
    }

    Color* mapPixels = LoadImageColors(mapImage);
    if (mapPixels == nullptr) {
        UnloadImage(mapImage);
        Unload();
        return false;
    }

    walkable_.resize(GRID_COLUMNS * GRID_ROWS);
    for (int y = 0; y < GRID_ROWS; ++y) {
        for (int x = 0; x < GRID_COLUMNS; ++x) {
            bool walkable = true;
            for (int pixelY = 0; pixelY < MAP_CELL_PIXELS && walkable; ++pixelY) {
                for (int pixelX = 0; pixelX < MAP_CELL_PIXELS; ++pixelX) {
                    const int mapX = x * MAP_CELL_PIXELS + pixelX;
                    const int mapY = y * MAP_CELL_PIXELS + pixelY;
                    const Color pixel = mapPixels[mapY * MAP_WIDTH + mapX];
                    if (pixel.a == 0 || pixel.r <= 32) {
                        walkable = false;
                        break;
                    }
                }
            }
            walkable_[y * GRID_COLUMNS + x] = walkable ? 1 : 0;
        }
    }

    UnloadImageColors(mapPixels);
    mapTexture_ = LoadTextureFromImage(mapImage);
    UnloadImage(mapImage);
    if (mapTexture_.id == 0) {
        Unload();
        return false;
    }
    SetTextureFilter(mapTexture_, TEXTURE_FILTER_POINT);

    Image snakeImage = LoadImage(
        "assets/snake_assets/Browser Games - Google Snake Game - Playable Characters & Objects - Snake (Pixel).png");
    if (snakeImage.data == nullptr) {
        UnloadImage(snakeImage);
        Unload();
        return false;
    }
    ImageColorReplace(&snakeImage, Color{ 0, 128, 128, 255 }, BLANK);
    snakeTexture_ = LoadTextureFromImage(snakeImage);
    UnloadImage(snakeImage);
    if (snakeTexture_.id == 0) {
        Unload();
        return false;
    }
    SetTextureFilter(snakeTexture_, TEXTURE_FILTER_POINT);

    appleTexture_ = LoadTexture("assets/snake_assets/AppleAnimation.png");
    if (appleTexture_.id == 0) {
        Unload();
        return false;
    }
    SetTextureFilter(appleTexture_, TEXTURE_FILTER_POINT);

    std::vector<Rectangle> appleFrames;
    appleFrames.reserve(11);
    for (int frame = 0; frame < 11; ++frame) {
        appleFrames.push_back({ static_cast<float>(frame * 16), 0.0f, 16.0f, 16.0f });
    }
    if (!appleAnimation_.AddClip("apple", std::move(appleFrames), 10.0f)) {
        Unload();
        return false;
    }

    Reset();
    return true;
}

void Snake::Update(float deltaTime) {
    appleAnimation_.Update(deltaTime);

    if (gameOver_ || won_) {
        if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER)) {
            Reset();
        }
        return;
    }

    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SPACE)) {
        paused_ = !paused_;
    }

    if (paused_) {
        return;
    }

    ReadInput();
    stepTimer_ += deltaTime;

    while (stepTimer_ >= STEP_INTERVAL && !gameOver_ && !won_) {
        stepTimer_ -= STEP_INTERVAL;
        Step();
    }
}

void Snake::Draw() {
    viewport_.Begin();
    ClearBackground(Color{ 18, 24, 34, 255 });

    DrawText("SNAKE", BOARD_X, 12, 22, RAYWHITE);
    DrawText(TextFormat("SCORE  %d", score_), BOARD_X + GRID_COLUMNS * CELL_SIZE - 110, 16, 16, RAYWHITE);

    const Rectangle board {
        static_cast<float>(BOARD_X),
        static_cast<float>(BOARD_Y),
        static_cast<float>(GRID_COLUMNS * CELL_SIZE),
        static_cast<float>(GRID_ROWS * CELL_SIZE)
    };
    const Rectangle mapSource { 0.0f, 0.0f, static_cast<float>(MAP_WIDTH), static_cast<float>(MAP_HEIGHT) };
    DrawTexturePro(mapTexture_, mapSource, board, { 0.0f, 0.0f }, 0.0f, WHITE);
    DrawRectangleLinesEx(board, 2.0f, Color{ 151, 171, 171, 255 });

    const Rectangle appleFrame = appleAnimation_.GetSourceRectangle();
    const Rectangle appleDestination {
        static_cast<float>(BOARD_X + food_.x * CELL_SIZE),
        static_cast<float>(BOARD_Y + food_.y * CELL_SIZE),
        static_cast<float>(CELL_SIZE),
        static_cast<float>(CELL_SIZE)
    };
    DrawTexturePro(appleTexture_, appleFrame, appleDestination, { 0.0f, 0.0f }, 0.0f, WHITE);

    for (int index = static_cast<int>(body_.size()) - 1; index >= 0; --index) {
        if (index == 0) {
            float rotation = 0.0f;
            if (direction_ == Direction::Down) {
                rotation = 90.0f;
            } else if (direction_ == Direction::Left) {
                rotation = 180.0f;
            } else if (direction_ == Direction::Up) {
                rotation = 270.0f;
            }
            DrawSegment(body_[index], SNAKE_HEAD_FRAME, rotation);
        } else {
            DrawSegment(body_[index], SNAKE_BODY_FRAME, 0.0f);
        }
    }

    DrawText("Move: arrows / WASD     Pause: P or Space", BOARD_X, 382, 14, Color{ 178, 192, 199, 255 });

    if (paused_ || gameOver_ || won_) {
        DrawRectangle(BOARD_X, BOARD_Y, GRID_COLUMNS * CELL_SIZE, GRID_ROWS * CELL_SIZE,
                      Color{ 8, 13, 20, 190 });

        const char* message = paused_ ? "PAUSED" : (won_ ? "YOU WIN" : "GAME OVER");
        const int messageWidth = MeasureText(message, 32);
        DrawText(message, (VIEW_WIDTH - messageWidth) / 2, 160, 32, RAYWHITE);

        const char* hint = paused_ ? "Press P or Space to continue" : "Press R or Enter to play again";
        const int hintWidth = MeasureText(hint, 16);
        DrawText(hint, (VIEW_WIDTH - hintWidth) / 2, 204, 16, Color{ 206, 216, 220, 255 });
    }

    viewport_.End();
    viewport_.Present();
}

void Snake::Unload() {
    viewport_.Unload();

    if (mapTexture_.id != 0) {
        UnloadTexture(mapTexture_);
        mapTexture_ = Texture2D{};
    }
    if (snakeTexture_.id != 0) {
        UnloadTexture(snakeTexture_);
        snakeTexture_ = Texture2D{};
    }
    if (appleTexture_.id != 0) {
        UnloadTexture(appleTexture_);
        appleTexture_ = Texture2D{};
    }

    walkable_.clear();
    body_.clear();
}

void Snake::Reset() {
    body_.clear();
    direction_ = Direction::Right;
    queuedDirection_ = Direction::Right;
    stepTimer_ = 0.0f;
    score_ = 0;
    paused_ = false;
    gameOver_ = false;
    won_ = false;

    int bestX = -1;
    int bestY = -1;
    int bestDistance = std::numeric_limits<int>::max();
    for (int y = 0; y < GRID_ROWS; ++y) {
        for (int x = 0; x + 2 < GRID_COLUMNS; ++x) {
            if (!IsCellWalkable({ x, y }) || !IsCellWalkable({ x + 1, y }) || !IsCellWalkable({ x + 2, y })) {
                continue;
            }

            const int distance = std::abs(x + 1 - GRID_COLUMNS / 2) + std::abs(y - GRID_ROWS / 2);
            if (distance < bestDistance) {
                bestDistance = distance;
                bestX = x;
                bestY = y;
            }
        }
    }

    if (bestX < 0) {
        gameOver_ = true;
        return;
    }

    body_ = { { bestX + 2, bestY }, { bestX + 1, bestY }, { bestX, bestY } };
    SpawnFood();
}

void Snake::ReadInput() {
    Direction requested = queuedDirection_;

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        requested = Direction::Up;
    } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        requested = Direction::Down;
    } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        requested = Direction::Left;
    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        requested = Direction::Right;
    }

    if (!IsOpposite(requested, direction_) && !IsOpposite(requested, queuedDirection_)) {
        queuedDirection_ = requested;
    }
}

void Snake::Step() {
    direction_ = queuedDirection_;
    const Cell offset = Offset(direction_);
    const Cell next { body_.front().x + offset.x, body_.front().y + offset.y };
    const bool eating = next == food_;

    if (!IsCellWalkable(next)) {
        gameOver_ = true;
        return;
    }

    const int collisionLength = static_cast<int>(body_.size()) - (eating ? 0 : 1);
    for (int index = 0; index < collisionLength; ++index) {
        if (body_[index] == next) {
            gameOver_ = true;
            return;
        }
    }

    body_.insert(body_.begin(), next);

    if (eating) {
        ++score_;
        SpawnFood();
    } else {
        body_.pop_back();
    }
}

void Snake::SpawnFood() {
    std::vector<Cell> available;
    available.reserve(GRID_COLUMNS * GRID_ROWS - body_.size());

    for (int y = 0; y < GRID_ROWS; ++y) {
        for (int x = 0; x < GRID_COLUMNS; ++x) {
            const Cell cell { x, y };
            if (IsCellWalkable(cell) && !Contains(cell)) {
                available.push_back(cell);
            }
        }
    }

    if (available.empty()) {
        won_ = true;
        return;
    }

    food_ = available[GetRandomValue(0, static_cast<int>(available.size()) - 1)];
}

bool Snake::Contains(Cell cell) const {
    return std::find(body_.begin(), body_.end(), cell) != body_.end();
}

bool Snake::IsCellWalkable(Cell cell) const {
    if (cell.x < 0 || cell.x >= GRID_COLUMNS || cell.y < 0 || cell.y >= GRID_ROWS) {
        return false;
    }

    const std::size_t index = static_cast<std::size_t>(cell.y * GRID_COLUMNS + cell.x);
    return index < walkable_.size() && walkable_[index] != 0;
}

bool Snake::IsOpposite(Direction first, Direction second) {
    return (first == Direction::Up && second == Direction::Down) ||
           (first == Direction::Down && second == Direction::Up) ||
           (first == Direction::Left && second == Direction::Right) ||
           (first == Direction::Right && second == Direction::Left);
}

Snake::Cell Snake::Offset(Direction direction) {
    switch (direction) {
        case Direction::Up: return { 0, -1 };
        case Direction::Down: return { 0, 1 };
        case Direction::Left: return { -1, 0 };
        case Direction::Right: return { 1, 0 };
    }

    return {};
}

void Snake::DrawSegment(Cell cell, Rectangle source, float rotation) const {
    const Rectangle destination {
        static_cast<float>(BOARD_X + cell.x * CELL_SIZE),
        static_cast<float>(BOARD_Y + cell.y * CELL_SIZE),
        static_cast<float>(CELL_SIZE),
        static_cast<float>(CELL_SIZE)
    };
    const Vector2 origin { CELL_SIZE * 0.5f, CELL_SIZE * 0.5f };
    DrawTexturePro(snakeTexture_, source, destination, origin, rotation, WHITE);
}
