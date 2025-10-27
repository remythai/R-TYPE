#include "InputHandler.hpp"

extern "C"
{
    ISystem* createSystem()
    {
        return new GameEngine::InputHandler();
    }

    void destroySystem(ISystem* system)
    {
        delete system;
    }

    const char* getSystemName()
    {
        return "InputHandler";
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