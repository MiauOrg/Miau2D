#pragma once

#include "Physics/GridPathfinder.hpp"

#include <cstddef>
#include <vector>

class Actor;

class GridPathFollower {
public:
    void SetPath(std::vector<GridCell> path);
    void Clear();
    void Update(Actor& actor);
    [[nodiscard]] bool IsActive() const;

private:
    std::vector<GridCell> path_;
    std::size_t nextCell_ = 0;
};
