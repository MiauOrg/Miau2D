#include "WanderingNPC.hpp"

#include "Physics/GridPathfinder.hpp"
#include "Scene.hpp"

WanderingNPC::WanderingNPC(unsigned int randomSeed)
    : brain_(randomSeed) {
}

void WanderingNPC::Update(Scene& scene, float deltaTime) {
    brain_.Update(*this, scene.GetPathfinder(), deltaTime);
    Actor::Update(scene, deltaTime);
}
