#include "Death.hpp"

extern "C"
{
    ISystem* createSystem()
    {
        return new GameEngine::Death();
    }

    void destroySystem(ISystem* system)
    {
        delete system;
    }

    const char* getSystemName()
    {
        return "Death";
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