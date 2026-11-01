#pragma once

#include "GridPathFollower.hpp"

#include <random>
#include <vector>

class Actor;
class GridPathfinder;

class WanderingBrain {
public:
    explicit WanderingBrain(unsigned int randomSeed = 1);
    void Update(Actor& actor, const GridPathfinder& pathfinder, float deltaTime);

private:
    GridPathFollower follower_;
    std::mt19937 random_;
    float waitRemaining_ = 0.0f;

    bool ChooseDestination(Actor& actor, const GridPathfinder& pathfinder);
};
