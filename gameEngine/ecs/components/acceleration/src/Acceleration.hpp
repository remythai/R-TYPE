#pragma once

#include "../../AComponent.hpp"

namespace GameEngine {
class Acceleration : public AComponent {
public:
	Acceleration ();
	~Acceleration ();
    
    void set_acceleration(float x, float y) {
        this->_x = x;
        this->_y = y;
    }
    float get_accelerationx() const {
        return this->_x;
    }
    float get_accelerationy() const {
        return this->_y;
    }
private:
    float _x;
    float _y;
};
}
