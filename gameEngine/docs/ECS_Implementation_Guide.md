# ECS Implementation Guide: Components and Systems

## Executive Summary

This guide provides comprehensive instructions for implementing Components and Systems within our ECS-based game engine. It establishes architectural patterns, design principles, and implementation standards to ensure consistency and performance across the codebase.

## Table of Contents

1. [Component Implementation](#component-implementation)
2. [System Implementation](#system-implementation)
3. [Architecture Integration](#architecture-integration)
4. [Best Practices](#best-practices)

---

## Component Implementation

### Fundamental Principles

**Components are pure data structures** with no logic or behavior. They represent a single aspect of an entity's state and should adhere to the following principles:

- **Data-Only**: No methods except constructors and simple getters/setters
- **POD-Compatible**: Prefer Plain Old Data structures when possible
- **Lightweight**: Minimize memory footprint (cache efficiency)
- **Self-Contained**: No dependencies on other components or systems
- **Copyable**: Support efficient copying and moving

### Component Structure

```cpp
#pragma once

#include "Component.hpp"

namespace GameEngine {

/**
 * @brief Component description
 * 
 * Explain what aspect of entity state this represents
 * and typical use cases.
 */
struct ComponentName : public Component<ComponentName> {
    // Data members only
    Type member1;
    Type member2;
    
    // Constructor with default values
    ComponentName(Type val1 = default, Type val2 = default)
        : member1(val1), member2(val2) {}
    
    // Required static metadata
    static constexpr const char* Name = "ComponentName";
    static constexpr const char* Version = "1.0.0";
    
    // Optional: Simple utility methods (no complex logic)
    void reset() {
        member1 = default;
        member2 = default;
    }
};

} // namespace GameEngine
```

### Implementation Steps

#### 1. Define Component Purpose

Before writing code, clearly define:
- **What data** does this component hold?
- **Which systems** will read/write this data?
- **What aspect** of entity behavior does it represent?

#### 2. Choose Data Members

Select the minimal set of data members needed:
```cpp
struct Acceleration : public Component<Acceleration> {
    float x, y;  // Only the essential data
    
    Acceleration(float val_x = 0, float val_y = 0) 
        : x(val_x), y(val_y) {}
    
    static constexpr const char* Name = "Acceleration";
    static constexpr const char* Version = "1.0.0";
};
```

**Considerations:**
- Use primitive types when possible (float, int, bool)
- Avoid pointers or references to other components
- Keep total size reasonable (<256 bytes ideally)
- Use default initialization for all members

#### 3. File Organization

**File Location**: `components/componentname/src/ComponentName.hpp`

**Naming Convention**:
- PascalCase for component names
- Descriptive, not abbreviated (prefer `Velocity` over `Vel`)
- Noun-based names (components are things, not actions)

#### 4. Documentation Requirements

Every component must include:
```cpp
/**
 * @brief One-line description
 * 
 * Detailed explanation:
 * - What this component represents
 * - Which systems typically use it
 * - Common usage patterns
 * - Any special considerations
 * 
 * @inherits Component<ComponentName> for ECS registration
 * 
 * @example
 * ```cpp
 * Entity entity = registry.create();
 * registry.emplace<ComponentName>(entity, value1, value2);
 * ```
 */
```

#### 5. Required Metadata

Every component **must** define these static members:

```cpp
static constexpr const char* Name = "ComponentName";
static constexpr const char* Version = "1.0.0";
```

These enable:
- Component identification in the registry
- Serialization and deserialization
- Version compatibility checking
- Dynamic loading from shared libraries

### Design Patterns for Components

#### Simple Data Component
For basic properties:
```cpp
struct Health : public Component<Health> {
    int current;
    int maximum;
    
    Health(int max = 100) : current(max), maximum(max) {}
    
    static constexpr const char* Name = "Health";
    static constexpr const char* Version = "1.0.0";
};
```

#### Stateful Component
When component needs internal state tracking:
```cpp
struct AnimationState : public Component<AnimationState> {
    int currentFrame;
    float elapsedTime;
    bool isPlaying;
    
    AnimationState() 
        : currentFrame(0), elapsedTime(0.0f), isPlaying(false) {}
    
    static constexpr const char* Name = "AnimationState";
    static constexpr const char* Version = "1.0.0";
};
```

#### Configuration Component
For entity behavior configuration:
```cpp
struct MovementConfig : public Component<MovementConfig> {
    float maxSpeed;
    float acceleration;
    float friction;
    
    MovementConfig(float speed = 100.0f, float accel = 50.0f, float fric = 0.9f)
        : maxSpeed(speed), acceleration(accel), friction(fric) {}
    
    static constexpr const char* Name = "MovementConfig";
    static constexpr const char* Version = "1.0.0";
};
```

#### Tag Component
For entity categorization (no data needed):
```cpp
struct Player : public Component<Player> {
    static constexpr const char* Name = "Player";
    static constexpr const char* Version = "1.0.0";
};

struct Enemy : public Component<Enemy> {
    static constexpr const char* Name = "Enemy";
    static constexpr const char* Version = "1.0.0";
};
```

### Common Component Types

**Transform Components**: Position, rotation, scale  
**Physics Components**: Velocity, acceleration, mass, collision boxes  
**Rendering Components**: Sprite data, color, layer, visibility  
**Gameplay Components**: Health, damage, score, inventory  
**AI Components**: Behavior state, target, patrol points  
**Audio Components**: Sound source, volume, pitch

---

## System Implementation

### Fundamental Principles

**Systems contain all game logic** and operate on entities with specific component combinations. Key principles:

- **Stateless When Possible**: Avoid internal state; use components for state
- **Single Responsibility**: Each system handles one specific concern
- **Component-Driven**: System logic is determined by component data
- **Frame-Based**: Executes once per game loop iteration
- **Registry-Dependent**: All entity/component access through Registry

### System Structure

Our engine uses the **CRTP (Curiously Recurring Template Pattern)** for systems:

```cpp
#pragma once
#include "System.hpp"
#include "../../../components/component1/src/Component1.hpp"
#include "../../../components/component2/src/Component2.hpp"

namespace GameEngine {

/**
 * @brief System description
 * 
 * Explain what logic this system implements and
 * how it modifies entity state.
 */
class SystemName : public System<SystemName> {
public:
    /**
     * @brief Constructs the system and declares component requirements.
     */
    SystemName() {
        // Define required components
        requireComponents<Component1, Component2>();
        
        // Optional: Set system name for debugging
        setName("SystemName");
        
        // Optional: Set execution priority
        priority = 100;  // Higher = runs earlier
    }
    
    /**
     * @brief System update logic
     * 
     * @param registry ECS registry for entity/component access
     * @param dt Delta time since last frame (seconds)
     */
    void onUpdate(Registry& registry, float dt);

private:
    // Helper methods and system-specific data
};

} // namespace GameEngine
```

### Implementation Steps

#### 1. Define System Responsibility

Clearly establish:
- **What logic** does this system implement?
- **Which components** does it require?
- **When** should it run relative to other systems?
- **What changes** does it make to entity state?

#### 2. Declare Component Requirements

In the constructor, specify which components entities must have:

```cpp
SystemName() {
    // Single component requirement
    requireComponents<Transform>();
    
    // Multiple component requirements
    requireComponents<Transform, Velocity>();
    
    // System must match entities with ALL listed components
    requireComponents<Transform, Sprite, Animator>();
}
```

**Important**: Only entities possessing **all** required components will be processed by this system.

#### 3. Set System Properties

Configure system behavior:

```cpp
SystemName() {
    requireComponents<Component>();
    
    // Human-readable name (for debugging/profiling)
    setName("SystemName");
    
    // Execution priority (default: 0)
    // Higher priority systems execute first
    priority = 100;
    
    // System can be disabled at runtime
    enabled = true;  // Default value
}
```

**Priority Guidelines**:
- Input handling: 200+
- Physics update: 100-199
- Gameplay logic: 50-99
- Animation/effects: 10-49
- Rendering: 0-9

#### 4. Implement Update Logic

The `onUpdate` method contains the core system logic. Use the `each()` method for iterating over entities:

**Standard Approach: Using `each()`**
```cpp
void onUpdate(Registry& registry, float dt) {
    // each() automatically unpacks components into lambda parameters
    registry.each<RequiredComponent1, RequiredComponent2>(
        [dt](Entity entity, auto& comp1, auto& comp2) {
            // Components are directly accessible
            comp1.value += comp2.value * dt;
        }
    );
}
```

**Key Benefits of `each()`:**
- Cleaner, more concise syntax
- Automatic component unpacking
- Direct access to entity ID
- No manual `get<>()` calls needed

#### 5. File Organization

**File Location**: `gameEngine/src/systems/systemname/SystemName.hpp`

**Naming Convention**:
- PascalCase for class names: `MovementSystem`, `RenderSystem`
- Action-based names describing what the system does
- Clear, descriptive (prefer `CollisionDetectionSystem` over `CollisionSystem`)

### System Design Patterns

#### Transform Pattern: Modifying Component Data

Most common pattern - read and modify components:

```cpp
void onUpdate(Registry& registry, float dt) {
    registry.each<Position, Velocity>(
        [dt](Entity entity, auto& position, auto& velocity) {
            position.pos.x += velocity.x * dt;
            position.pos.y += velocity.y * dt;
        }
    );
}
```

#### Query Pattern: Entity Filtering

When you need additional conditional logic beyond component requirements:

```cpp
void onUpdate(Registry& registry, float dt) {
    registry.each<Health>(
        [&registry](Entity entity, auto& health) {
            if (health.current <= 0) {
                registry.destroy(entity);
            }
        }
    );
}
```

#### Multi-Phase Pattern: Complex Processing

When logic requires multiple passes:

```cpp
void onUpdate(Registry& registry, float dt) {
    // Phase 1: Deceleration
    registry.each<Velocity>([dt](Entity e, auto& vel) {
        vel.x *= 0.25f;  // 75% reduction
        vel.y *= 0.25f;
    });
    
    // Phase 2: Acceleration
    registry.each<Velocity, Acceleration>([dt](Entity e, auto& vel, auto& acc) {
        vel.x = std::clamp(vel.x + acc.x, -vel.speedMax, vel.speedMax);
        vel.y = std::clamp(vel.y + acc.y, -vel.speedMax, vel.speedMax);
    });
    
    // Phase 3: Position update
    registry.each<Position, Velocity>([dt](Entity e, auto& pos, auto& vel) {
        pos.pos.x += vel.x;
        pos.pos.y += vel.y;
    });
}
```

#### Deferred Modification Pattern: Safe Entity Destruction

Avoid modifying entity structure during iteration:

```cpp
void onUpdate(Registry& registry, float dt) {
    // Collect entities to destroy
    std::vector<Entity> toDestroy;
    
    registry.each<Health>([&toDestroy](Entity entity, auto& health) {
        if (health.current <= 0) {
            toDestroy.push_back(entity);
        }
    });
    
    // Destroy after iteration completes
    for (auto entity : toDestroy) {
        registry.destroy(entity);
    }
}
```

### Complete System Example

Here's a complete implementation of the Motion system from our engine:

```cpp
#pragma once

#include <algorithm>
#include "../../../components/position/src/Position.hpp"
#include "../../../components/velocity/src/Velocity.hpp"
#include "../../../components/acceleration/src/Acceleration.hpp"
#include "../../../components/renderable/src/Renderable.hpp"
#include "../../../components/collider/src/Collider.hpp"
#include "../../../ecs/Registry.hpp"
#include "../../../ecs/System.hpp"

namespace GameEngine {

/**
 * @class Motion
 * @brief System that simulates entity movement using physics principles.
 *
 * The Motion system implements a simplified physics engine that handles:
 * 1. Deceleration: Velocity naturally dampens when acceleration is zero
 * 2. Acceleration: Applies acceleration forces to velocity each frame
 * 3. Speed clamping: Constrains velocity to maximum speed limits
 * 4. Position updates: Translates entities based on current velocity
 * 5. Boundary clamping: Keeps entities within screen boundaries
 */
class Motion : public System<Motion> {
public:
    Motion() {
        requireComponents
            Position, Velocity, Acceleration, 
            Renderable, Collider>();
        setName("Motion");
        priority = 100;
    }
    
    void onUpdate(Registry& registry, float dt) {
        registry.each<Position, Velocity, Acceleration, Renderable, Collider>(
            [dt](Entity e, auto& pos, auto& vel, auto& acc, 
                 auto& render, auto& collider) {
                // Phase 1: Deceleration (75% reduction)
                vel.x = vel.x > 0 
                    ? std::max(vel.x * 0.25f, 0.0f)
                    : std::min(vel.x * 0.25f, 0.0f);
                vel.y = vel.y > 0 
                    ? std::max(vel.y * 0.25f, 0.0f)
                    : std::min(vel.y * 0.25f, 0.0f);
                
                // Phase 2: Acceleration (apply forces and clamp)
                vel.x = std::clamp(vel.x + acc.x, -vel.speedMax, vel.speedMax);
                vel.y = std::clamp(vel.y + acc.y, -vel.speedMax, vel.speedMax);
                
                // Phase 3: Position update (translate and constrain)
                pos.pos.x = std::clamp(
                    pos.pos.x + vel.x, 0.0f,
                    render.screenSizeX - collider.size.x);
                pos.pos.y = std::clamp(
                    pos.pos.y + vel.y, 0.0f,
                    render.screenSizeY - collider.size.y);
            }
        );
    }
};

} // namespace GameEngine
```

---

## Architecture Integration

### Component Registration

Components are automatically registered when first used:
```cpp
// In system constructor or elsewhere
requireComponents<Transform>();  // Auto-registers Transform component
```

The `ComponentRegistry` maintains a mapping between component types and IDs.

### System Registration

Systems are added to the Registry:
```cpp
// In main.cpp or game initialization
Registry registry;

registry.addSystem<MovementSystem>();
registry.addSystem<RenderSystem>();
registry.addSystem<CollisionSystem>();
```

Systems execute in priority order each frame.

### Entity Creation with Components

```cpp
// Create entity
Entity player = registry.create();

// Add components using emplace
registry.emplace<Position>(player, 100.0f, 100.0f);
registry.emplace<Velocity>(player, 10.0f, 50.0f, 0.0f);
registry.emplace<Sprite>(player, "player.png", 1);
```

### System Execution Flow

```cpp
// Game loop
while (running) {
    float dt = calculateDeltaTime();
    
    // Update all systems in priority order
    registry.update(dt);
    
    // Systems automatically process matching entities
}
```

---

## Best Practices

### Component Design

**DO:**
- ✅ Keep components small and focused
- ✅ Use value types (no pointers when avoidable)
- ✅ Provide meaningful default values
- ✅ Document expected value ranges
- ✅ Always inherit from `Component<T>`
- ✅ Always define `Name` and `Version` static members

**DON'T:**
- ❌ Add methods beyond simple utilities
- ❌ Reference other components directly
- ❌ Allocate dynamic memory in constructors
- ❌ Make components larger than necessary
- ❌ Use inheritance for components (except `Component<T>`)

### System Design

**DO:**
- ✅ Keep systems stateless when possible
- ✅ Use descriptive names with clear purpose
- ✅ Document system execution order requirements
- ✅ Profile performance-critical systems
- ✅ Handle edge cases (empty views, invalid data)
- ✅ Use `each()` for cleaner iteration code

**DON'T:**
- ❌ Create systems that do too much
- ❌ Modify entity structure during iteration
- ❌ Store entity references across frames
- ❌ Assume component data is valid without checking
- ❌ Couple systems together unnecessarily

### Performance Considerations

**Cache Efficiency:**
- Access components sequentially in hot loops
- Minimize component size for better cache line utilization
- Batch similar operations together
- Use `each()` which optimizes for cache-friendly access

**System Ordering:**
- Systems reading data should run before those writing it
- Physics before rendering
- Input before gameplay logic
- Use priority values to enforce ordering

**Entity Management:**
- Reuse entities when possible (object pooling)
- Destroy entities in batches outside iteration
- Minimize component add/remove operations per frame
- Use the deferred modification pattern for safety

---

## Conclusion

Implementing components and systems following these guidelines ensures:
- **Consistent architecture** across the codebase
- **High performance** through cache-friendly design
- **Maintainability** via clear separation of concerns
- **Flexibility** for rapid feature development
- **Type safety** through CRTP and compile-time checks

Always prioritize clarity and simplicity in component/system design. Complex logic should be broken into multiple simple systems rather than creating monolithic systems.

**Author**: Antton Ducos
**Contact**: antton.ducos@epitech.eu