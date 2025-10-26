#include "Motion.hpp"
#include <memory>


extern "C" {
    ISystem* createMotionSystem() {
        return new GameEngine::Motion();
    }

    void destroyMotionSystem(ISystem* system) {
        delete system;
    }

    const char* getSystemName() {
        return "Motion";
    }

    const char* getSystemVersion() {
        return "1.0.0";
    }


    int getSystemDefaultPriority() {
        return 100;
    }

    const char* getRequiredComponents() {
        return "Position,Velocity,Acceleration,Renderable";
    }
} // extern "C"