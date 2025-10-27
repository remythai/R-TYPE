#include "Animation.hpp"

extern "C"
{
    ISystem* createSystem()
    {
        return new GameEngine::Animation();
    }

    void destroySystem(ISystem* system)
    {
        delete system;
    }

    const char* getSystemName()
    {
        return "Animation";
    }

    const char* getSystemVersion()
    {
        return "1.0.0";
    }

    int getSystemDefaultPriority()
    {
        return 100;
    }

}  // extern "C"