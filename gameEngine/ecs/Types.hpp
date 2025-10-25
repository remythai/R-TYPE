#pragma once

#include <bitset>
#include <cstdint>

/**
 * @file Types.hpp
 * @brief Core type definitions used throughout the ECS (Entity Component
 * System).
 *
 * This header defines essential type aliases and constants for component,
 * system, and signature management. These definitions provide a consistent
 * foundation for entity, component, and system identification and interaction.
 */

/**
 * @typedef ComponentID
 * @brief Unique identifier for each component type.
 *
 * Each registered component (e.g., Transform, Velocity, Health)
 * receives a unique `ComponentID` assigned by the ComponentRegistry.
 */
using ComponentID = uint32_t;

/**
 * @typedef SystemID
 * @brief Unique identifier for each system type.
 *
 * Each registered system (e.g., RenderSystem, PhysicsSystem)
 * is assigned a unique `SystemID` by the Registry.
 */
using SystemID = uint32_t;

/**
 * @def MAX_COMPONENTS
 * @brief Maximum number of distinct component types supported by the ECS.
 *
 * This value defines the size of the bitset used to represent component
 * signatures. Adjust if your project requires more components.
 */
constexpr size_t MAX_COMPONENTS = 128;

/**
 * @typedef ComponentSignature
 * @brief Bitset representing which components an entity or system has.
 *
 * Each bit corresponds to a component type (`ComponentID`).
 * - For entities: which components are attached.
 * - For systems: which components are required.
 *
 * Example:
 * ```
 * ComponentSignature sig;
 * sig.set(TransformID);
 * sig.set(VelocityID);
 * ```
 */
using ComponentSignature = std::bitset<MAX_COMPONENTS>;
