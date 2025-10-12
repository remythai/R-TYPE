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

namespace GameEngine {

/**
 * @brief Component description
 * 
 * Explain what aspect of entity state this represents
 * and typical use cases.
 */
struct ComponentName {
    // Data members only
    Type member1;
    Type member2;
    
    // Constructor with default values
    ComponentName(Type val1 = default, Type val2 = default)
        : member1(val1), member2(val2) {}
    
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
struct Acceleration {
    float x, y;  // Only the essential data
    
    Acceleration(float val_x = 0, float val_y = 0) 
        : x(val_x), y(val_y) {}
};
```

**Considerations:**
- Use primitive types when possible (float, int, bool)
- Avoid pointers or references to other components
- Keep total size reasonable (<256 bytes ideally)
- Use default initialization for all members

#### 3. File Organization

**File Location**: `include/Components/ComponentName.hpp`

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
 */
```

### Design Patterns for Components

#### Simple Data Component
For basic properties:
```cpp
struct Health {
    int current;
    int maximum;
    
    Health(int max = 100) : current(max), maximum(max) {}
};
```

#### Stateful Component
When component needs internal state tracking:
```cpp
struct AnimationState {
    int currentFrame;
    float elapsedTime;
    bool isPlaying;
    
    AnimationState() 
        : currentFrame(0), elapsedTime(0.0f), isPlaying(false) {}
};
```

#### Configuration Component
For entity behavior configuration:
```cpp
struct MovementConfig {
    float maxSpeed;
    float acceleration;
    float friction;
    
    MovementConfig(float speed = 100.0f, float accel = 50.0f, float fric = 0.9f)
        : maxSpeed(speed), acceleration(accel), friction(fric) {}
};
```

#### Tag Component
For entity categorization (no data needed):
```cpp
struct Player {};  // Empty struct used as a tag
struct Enemy {};
struct Projectile {};
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
#include "Components/RequiredComponent1.hpp"
#include "Components/RequiredComponent2.hpp"

namespace GameEngine {

/**
 * @brief System description
 * 
 * Explain what logic this system implements and
 * how it modifies entity state.
 */
class SystemName : public System<SystemName> {
public:
    SystemName() {
        // Define required components
        requireComponents<Component1, Component2>();
        
        // Set system name for debugging/profiling
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

The `onUpdate` method contains the core system logic. There are two primary approaches for iterating over entities:

**Approach 1: Using `view()` with manual iteration**
```cpp
void onUpdate(Registry& registry, float dt) {
    // 1. Get view of all matching entities
    auto view = registry.view<RequiredComponent1, RequiredComponent2>();
    
    // 2. Iterate over entities
    for (auto entity : view) {
        // 3. Access components
        auto& comp1 = view.get<RequiredComponent1>(entity);
        auto& comp2 = view.get<RequiredComponent2>(entity);
        
        // 4. Implement system logic
        comp1.value += comp2.value * dt;
    }
}
```

**Approach 2: Using `each()` for cleaner syntax**
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

**Key Differences:**

| Aspect | `view()` + manual loop | `each()` method |
|--------|------------------------|-----------------|
| Syntax | More verbose | Cleaner, more concise |
| Component access | Explicit `view.get<>()` | Automatic unpacking |
| Entity access | Always available | Available as parameter |
| Performance | Identical | Identical |
| Flexibility | More control | Less boilerplate |

**When to use `each()`:**
- ✅ When you need all required components for each entity
- ✅ For simple, straightforward logic
- ✅ When readability is prioritized

**When to use `view()`:**
- ✅ When you need additional filtering logic
- ✅ For complex multi-view interactions
- ✅ When you need more control over iteration
- ✅ For early loop termination based on conditions

#### 5. File Organization

**File Location**: `include/Systems/SystemName.hpp` and `src/Systems/SystemName.cpp`

**Naming Convention**:
- PascalCase with "System" suffix: `MovementSystem`, `RenderSystem`
- Action-based names describing what the system does
- Clear, descriptive (prefer `CollisionDetectionSystem` over `CollisionSystem`)

### System Design Patterns

#### Transform Pattern: Modifying Component Data

Most common pattern - read and modify components. Can use either iteration method:

**Using `view()`:**
```cpp
void onUpdate(Registry& registry, float dt) {
    auto view = registry.view<Transform, Velocity>();
    
    for (auto entity : view) {
        auto& transform = view.get<Transform>(entity);
        auto& velocity = view.get<Velocity>(entity);
        
        transform.x += velocity.x * dt;
        transform.y += velocity.y * dt;
    }
}
```

**Using `each()` (recommended for simple cases):**
```cpp
void onUpdate(Registry& registry, float dt) {
    registry.each<Transform, Velocity>(
        [dt](Entity entity, auto& transform, auto& velocity) {
            transform.x += velocity.x * dt;
            transform.y += velocity.y * dt;
        }
    );
}
```

#### Query Pattern: Entity Filtering

When you need additional conditional logic beyond component requirements:

**Using `view()` (better for filtering):**
```cpp
void onUpdate(Registry& registry, float dt) {
    auto view = registry.view<Health>();
    
    for (auto entity : view) {
        auto& health = view.get<Health>(entity);
        
        // Additional filtering
        if (health.current <= 0) {
            registry.destroyEntity(entity);
        }
    }
}
```

**Using `each()` with conditional logic:**
```cpp
void onUpdate(Registry& registry, float dt) {
    registry.each<Health>(
        [&registry](Entity entity, auto& health) {
            if (health.current <= 0) {
                registry.destroyEntity(entity);
            }
        }
    );
}
```

**Note:** For complex filtering, `view()` often provides better readability.

#### Multi-View Pattern: Cross-Entity Interactions

When entities interact with each other:
```cpp
void onUpdate(Registry& registry, float dt) {
    auto projectiles = registry.view<Projectile, Transform>();
    auto enemies = registry.view<Enemy, Transform, Health>();
    
    // Check projectile-enemy collisions
    for (auto proj : projectiles) {
        auto& projTransform = projectiles.get<Transform>(proj);
        
        for (auto enemy : enemies) {
            auto& enemyTransform = enemies.get<Transform>(enemy);
            auto& enemyHealth = enemies.get<Health>(enemy);
            
            if (checkCollision(projTransform, enemyTransform)) {
                enemyHealth.current -= 10;
                registry.destroyEntity(proj);
                break;
            }
        }
    }
}
```

#### Deferred Modification Pattern: Safe Entity Destruction

Avoid modifying entity structure during iteration:
```cpp
void onUpdate(Registry& registry, float dt) {
    auto view = registry.view<Health>();
    
    // Collect entities to destroy
    std::vector<Entity> toDestroy;
    
    for (auto entity : view) {
        auto& health = view.get<Health>(entity);
        if (health.current <= 0) {
            toDestroy.push_back(entity);
        }
    }
    
    // Destroy after iteration completes
    for (auto entity : toDestroy) {
        registry.destroyEntity(entity);
    }
}
```

---

## Architecture Integration

### Component Registration

Components are automatically registered when first used:
```cpp
// In system constructor or elsewhere
requireComponents<Transform>();  // Auto-registers Transform
```

The `ComponentRegistry` maintains a mapping between component types and IDs.

### System Registration

Systems are added to the Registry:
```cpp
// In main.cpp or game initialization
Registry registry;

registry.registerSystem<MovementSystem>();
registry.registerSystem<RenderSystem>();
registry.registerSystem<CollisionSystem>();
```

Systems execute in priority order each frame.

### Entity Creation with Components

```cpp
// Create entity
Entity player = registry.createEntity();

// Add components
registry.addComponent<Transform>(player, 100.0f, 100.0f);
registry.addComponent<Velocity>(player, 50.0f, 0.0f);
registry.addComponent<Sprite>(player, "player.png", 1);
```

### System Execution Flow

```cpp
// Game loop
while (running) {
    float dt = calculateDeltaTime();
    
    // Update all systems in priority order
    registry.updateSystems(dt);
    
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
- ✅ Use namespaces consistently

**DON'T:**
- ❌ Add methods beyond simple utilities
- ❌ Reference other components directly
- ❌ Allocate dynamic memory in constructors
- ❌ Make components larger than necessary
- ❌ Use inheritance for components

### System Design

**DO:**
- ✅ Keep systems stateless when possible
- ✅ Use descriptive names
- ✅ Document system execution order requirements
- ✅ Profile performance-critical systems
- ✅ Handle edge cases (empty views, null data)

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

**System Ordering:**
- Systems reading data should run before those writing it
- Physics before rendering
- Input before gameplay logic

**Entity Management:**
- Reuse entities when possible (object pooling)
- Destroy entities in batches outside iteration
- Minimize component add/remove operations per frame

---

## Conclusion

Implementing components and systems following these guidelines ensures:
- **Consistent architecture** across the codebase
- **High performance** through cache-friendly design
- **Maintainability** via clear separation of concerns
- **Flexibility** for rapid feature development

Always prioritize clarity and simplicity in component/system design. Complex logic should be broken into multiple simple systems rather than creating monolithic systems.