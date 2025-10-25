#pragma once

#include <chrono>
#include <ratio>

#include "../../../components/renderable/src/Renderable.hpp"
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"

namespace GameEngine {
class Animation : public System<Animation>
{
   private:
    std::chrono::time_point<
        std::chrono::steady_clock,
        std::chrono::duration<long, std::ratio<1, 1000000000>>>
        startPoint = std::chrono::steady_clock::now();

   public:
    Animation()
    {
        requireComponents<GameEngine::Renderable>();
    }

    void onUpdate(Registry& registry, float dt)
    {
        updateCount++;
        auto currentTimePoint = std::chrono::steady_clock::now();
        auto deltaTime = currentTimePoint - startPoint;

        registry.each<Renderable>([dt, deltaTime](auto e, Renderable& render) {
            if (render.rectPos.size() != 0)
                render.currentRectPos =
                    render.rectPos
                        [std::chrono::duration_cast<std::chrono::milliseconds>(
                             deltaTime)
                             .count() /
                         render.frameDuration % render.rectPos.size()];
        });
    }
    int updateCount = 0;
};
}  // namespace GameEngine
