#pragma once

#include <cstdint>
#include "../../../Registry.hpp"
#include "../../../System.hpp"
#include "../../../components/health/src/Health.hpp"
#include "../../../components/damage/src/Damage.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/velocity/src/Velocity.hpp"
#include "../../../components/acceleration/src/Acceleration.hpp"

namespace GameEngine {
    class InputHandlerSystem : public System<InputHandlerSystem> {
    public:
        InputHandlerSystem() {
            requireComponents<GameEngine::InputControlled, GameEngine::Acceleration>();
        }
        
        void onUpdate(Registry& registry, float dt) {
            updateCount++;
            registry.each<InputControlled, Acceleration>([dt, &registry](auto e, InputControlled& inputs, Acceleration& acceleration) {
                float accelerationValue = 10.0;
                acceleration.x = 0;
                acceleration.y = 0;
                for (auto &it : inputs.inputs) {
                    switch (it) {
                        case 0:
                            uint32_t e = registry.create();
                            registry.emplace<GameEngine::Health>(e, 1, 1);
                            registry.emplace<GameEngine::Damage>(e, 1);
                            registry.emplace<GameEngine::Velocity>(e, 1000, 1000);
                            registry.emplace<GameEngine::Acceleration>(e, 1000);
                            GameEngine::Position playerPos = registry.get<GameEngine::Position>(e);
                            registry.emplace<GameEngine::Position>(e, playerPos.x, playerPos.y);
                            break;
                        case 1:
                            acceleration.y = accelerationValue;
                            break;
                        case 2:
                            acceleration.x = -accelerationValue;
                            break;
                        case 3:
                            acceleration.y = -accelerationValue;
                            break;
                        case 4:
                            acceleration.x = accelerationValue;
                            break;
                        default:
                            break;
                    }
                }
            });
        }
        
        int updateCount = 0;
    };
}
