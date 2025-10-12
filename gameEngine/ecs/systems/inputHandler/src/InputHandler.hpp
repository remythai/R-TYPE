#pragma once

#include <cstdint>
#include "../../../Registry.hpp"
#include "../../../System.hpp"
#include "../../../components/health/src/Health.hpp"
#include "../../../components/damage/src/Damage.hpp"
#include "../../../components/position/src/Position.hpp"
#include "../../../components/velocity/src/Velocity.hpp"
#include "../../../components/acceleration/src/Acceleration.hpp"
#include "../../../components/inputControlled/src/InputControlled.hpp"
#include "../../../components/renderable/src/Renderable.hpp"

namespace GameEngine {
    class InputHandlerSystem : public System<InputHandlerSystem> {
    public:
        InputHandlerSystem() {
            requireComponents<GameEngine::InputControlled, GameEngine::Acceleration>();
        }
        
        void onUpdate(Registry& registry, float dt) {
            updateCount++;
            registry.each<InputControlled, Acceleration>([dt, &registry](auto e, InputControlled& inputs, Acceleration& acceleration) {
                float accelerationValue = 5.0;
                GameEngine::Position playerPos;
                uint32_t shoot = -1;
                acceleration.x = 0;
                acceleration.y = 0;
                for (auto &it : inputs.inputs) {
                    switch (it) {
                        case 0:
                            acceleration.y = accelerationValue;
                            break;
                        case 1:
                            acceleration.y = -accelerationValue;
                            break;
                        case 2:
                            acceleration.x = -accelerationValue;
                            break;
                        case 3:
                            acceleration.x = accelerationValue;
                            break;
                        case 4:
                            shoot = registry.create();
                            registry.emplace<GameEngine::Renderable>(shoot, 1920.0, 1080.0, "sprites/playerProjectiles.png", vec2{0.0f, 0.0f}, vec2{22.28f, 22.28f}, 3, 0, 0.05f);
                            registry.emplace<GameEngine::Health>(shoot, 1, 1);
                            registry.emplace<GameEngine::Damage>(shoot, 1);
                            registry.emplace<GameEngine::Velocity>(shoot, 1000.0, 1000.0);
                            registry.emplace<GameEngine::Acceleration>(shoot, 1000.0);
                            playerPos = registry.get<GameEngine::Position>(e);
                            registry.emplace<GameEngine::Position>(shoot, playerPos.x, playerPos.y);
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
