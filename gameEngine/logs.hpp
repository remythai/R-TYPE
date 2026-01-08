#pragma once
#include <iostream>

#ifndef ECS_DISABLE_LOGS
    #define ECS_LOG(x) do { std::cout << x << std::endl; } while(0)
#else
    #define ECS_LOG(x) do {} while(0)
#endif
