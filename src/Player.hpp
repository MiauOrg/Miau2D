#pragma once

#include "Actor.hpp"
#include "GridPathFollower.hpp"
#include "PlayerInput.hpp"
#include "TouchInput.hpp"

class Scene;

class Player final : public Actor {
public:
    void Update(Scene& scene, float deltaTime) override;

private:
    PlayerInput input_;
    TouchInput touchInput_;
    GridPathFollower touchPath_;
};
