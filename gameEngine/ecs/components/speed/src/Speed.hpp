#pragma once
#include "../../AComponent.hpp"

namespace GameEngine {
class Speed : public AComponent {
public:
	Speed ();
	~Speed ();
    
    void set_speed(float x, float y) {
        this->_x = x;
        this->_y = y;
    }
    float get_speedx() const {
        return this->_x;
    }
    float get_speedy() const {
        return this->_y;
    }
private:
    float _x;
    float _y;
};
}
