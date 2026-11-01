#include "Game.hpp"
#include "Snake.hpp"

std::unique_ptr<Game> CreateGame() {
    return std::make_unique<Snake>();
}
