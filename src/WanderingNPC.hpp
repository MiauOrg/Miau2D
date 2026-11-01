#pragma once

#include "Actor.hpp"
#include "WanderingBrain.hpp"

class Scene;

class WanderingNPC final : public Actor {
public:
    explicit WanderingNPC(unsigned int randomSeed = 1);
    void Update(Scene& scene, float deltaTime) override;

private:
    WanderingBrain brain_;
};
