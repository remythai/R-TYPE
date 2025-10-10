#pragma once

namespace GameEngine {
    struct GameClock {
        float totalTime = 0.0f;
        float fixedDeltaTime = 1.0f / 60.0f;
        uint64_t frameCount = 0;
        float timeScale = 1.0f;
        
        float accumulator = 0.0f;

        int update(float realDt) {
            accumulator += realDt * timeScale;
            
            int steps = 0;
            const int maxSteps = 5;
            
            while (accumulator >= fixedDeltaTime && steps < maxSteps) {
                totalTime += fixedDeltaTime;
                frameCount++;
                accumulator -= fixedDeltaTime;
                steps++;
            }
            
            return steps;
        }

        float getInterpolationAlpha() const {
            return accumulator / fixedDeltaTime;
        }
        
        float getFixedDeltaTime() const { 
            return fixedDeltaTime * timeScale; 
        }
    };
}