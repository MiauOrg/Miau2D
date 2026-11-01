#pragma once

#include "Actor.hpp"

class Scene;

class Player final : public Actor {
public:
    void Update(Scene& scene, float deltaTime) override;

private:
    bool preferHorizontal_ = true;
};
