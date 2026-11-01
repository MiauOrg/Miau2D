#include "Game.hpp"
#include "RPG.hpp"

std::unique_ptr<Game> CreateGame() {
    return std::make_unique<RPG>();
}
