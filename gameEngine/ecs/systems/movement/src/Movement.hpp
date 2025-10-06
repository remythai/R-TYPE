#pragma once

#include "../../ASystem.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/speed/src/Speed.hpp"
#include <memory>

namespace GameEngine {
class Movement : public ASystem {
public:
	Movement ();
	~Movement ();
    
    void compute(std::unique_ptr<Position> &, std::unique_ptr<Speed> &);
private:
};
}
