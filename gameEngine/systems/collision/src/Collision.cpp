#include "Collision.hpp"

extern "C"
{
    ISystem* createSystem()
    {
        return new GameEngine::Collision();
    }

    void destroySystem(ISystem* system)
    {
        delete system;
    }

    const char* getSystemName()
    {
        return "Collision";
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