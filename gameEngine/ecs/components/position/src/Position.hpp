#pragma once

#include "../../AComponent.hpp"

namespace GameEngine {
class Position : public AComponent {
public:
	Position ();
	~Position ();
    
    void set_position(float x, float y) {
        this->_x = x;
        this->_y = y;
    }
    float get_positionx() const {
        return this->_x;
    }
    float get_positiony() const {
        return this->_y;
    }
private:
    float _x;
    float _y;
};
}
