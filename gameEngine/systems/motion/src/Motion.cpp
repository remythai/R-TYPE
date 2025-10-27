#include "Motion.hpp"

extern "C"
{
    ISystem* createSystem()
    {
        return new GameEngine::Motion();
    }

    void destroySystem(ISystem* system)
    {
        delete system;
    }

    const char* getSystemName()
    {
        return "Motion";
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