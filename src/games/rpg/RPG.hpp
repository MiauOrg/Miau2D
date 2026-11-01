#pragma once

#include "Game.hpp"
#include "Scene.hpp"

class RPG final : public Game {
public:
    [[nodiscard]] GameConfig Config() const override;
    bool Load() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Unload() override;

private:
    Scene scene_;
};
