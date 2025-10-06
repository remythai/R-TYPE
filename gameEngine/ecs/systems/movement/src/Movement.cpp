#include "Movement.hpp"
#include <memory>

void GameEngine::Movement::Movement::compute(std::unique_ptr<GameEngine::Position> &Position, std::unique_ptr<GameEngine::Speed> &Speed) {
    Position->set_position(Position->get_positionx() + Speed->get_speedx(), Position->get_positiony() + Speed->get_speedy());
}
