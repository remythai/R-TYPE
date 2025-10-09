#pragma once

#include "../../../Registry.hpp"
#include "../../../System.hpp"

namespace GameEngine {
    class InputHandlerSystem : public System<InputHandlerSystem> {
    public:
        InputHandlerSystem() {
            requireComponents<GameEngine::InputControlled, GameEngine::Acceleration>();
        }
        
        void onUpdate(Registry& registry, float dt) {
            updateCount++;
            registry.each<InputControlled, Acceleration>([dt](auto e, InputControlled& inputs, Acceleration& acceleration) {
                float accelerationValue = 10.0;
                for (auto &it : inputs.inputs) {
                    switch (it) {
                        case 0:
                            // registry add entity (shoot)
                        case 1:
                            acceleration.y = accelerationValue;
                        case 2:
                            acceleration.x = -accelerationValue;
                        case 3:
                            acceleration.y = -accelerationValue;
                        case 4:
                            acceleration.x = accelerationValue;
                        default:
                            break;
                    }
                }
            });
        }
        
        int updateCount = 0;
    };
}
