#include "../../AComponent.hpp"

namespace GameEngine {
class Position : public AComponent {
public:
	Position ();
	~Position ();
    
    void set_position(float x, float y);
    float get_positionx() const;
    float get_positiony() const;
private:
    float _x;
    float _y;
};
}
