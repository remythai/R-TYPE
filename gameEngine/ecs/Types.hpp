#pragma once

#include <cstdint>
#include <bitset>

using ComponentID = uint32_t;
using SystemID = uint32_t;

constexpr size_t MAX_COMPONENTS = 128;
using ComponentSignature = std::bitset<MAX_COMPONENTS>;