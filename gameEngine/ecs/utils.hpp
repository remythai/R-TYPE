#pragma once

/**
 * @struct vec2
 * @brief 2D vector structure for position, velocity, and directional
 * calculations.
 *
 * Represents a two-dimensional vector with X and Y components.
 * Provides basic vector arithmetic operations commonly used in:
 * - Position tracking
 * - Velocity calculations
 * - Direction vectors
 * - Offset transformations
 *
 * @details
 * **Operator Overloads:**
 * - `operator+`: Creates new vector from addition
 * - `operator+=`: Mutates vector by adding another
 *
 * **Design Rationale:**
 * - Plain struct for trivial copyability
 * - No virtual functions for cache efficiency
 * - Direct member access for performance
 *
 * @performance
 * - Trivially copyable: efficient pass-by-value
 * - No heap allocations
 * - Inline-friendly operations
 *
 * @example
 * ```cpp
 * vec2 position(100, 200);
 * vec2 velocity(5, -3);
 * position += velocity;  // position is now (105, 197)
 *
 * vec2 sum = position + velocity;  // sum is (110, 194)
 * ```
 */
struct vec2
{
    /**
     * @brief X component of the vector.
     *
     * Represents horizontal position, velocity, or direction.
     * Positive values typically indicate rightward direction.
     */
    float x;

    /**
     * @brief Y component of the vector.
     *
     * Represents vertical position, velocity, or direction.
     * Positive values typically indicate downward direction in screen
     * coordinates.
     */
    float y;

    /**
     * @brief Default constructor initializing to origin (0, 0).
     *
     * Creates a zero vector, useful as a neutral element in vector arithmetic.
     *
     * @post x = 0, y = 0
     */
    vec2() : x(0), y(0) {}

    /**
     * @brief Parameterized constructor with explicit component values.
     *
     * @param x X component value.
     * @param y Y component value.
     *
     * @post Vector initialized with specified components.
     */
    vec2(float x, float y) : x(x), y(y) {}

    /**
     * @brief Vector addition operator (non-mutating).
     *
     * Creates a new vector representing the sum of this vector and another.
     * Original vectors remain unchanged.
     *
     * @param other Vector to add.
     * @return New vector with summed components.
     *
     * @details
     * Mathematical operation: `(x1, y1) + (x2, y2) = (x1 + x2, y1 + y2)`
     *
     * @example
     * ```cpp
     * vec2 a(10, 20);
     * vec2 b(5, 15);
     * vec2 c = a + b;  // c = (15, 35), a and b unchanged
     * ```
     */
    vec2 operator+(const vec2& other) const
    {
        return vec2(x + other.x, y + other.y);
    }

    /**
     * @brief Vector addition assignment operator (mutating).
     *
     * Adds another vector to this vector, modifying this vector in place.
     *
     * @param other Vector to add.
     * @return Reference to this vector after modification.
     *
     * @post This vector's components are increased by other's components.
     *
     * @example
     * ```cpp
     * vec2 position(100, 200);
     * vec2 velocity(5, -3);
     * position += velocity;  // position becomes (105, 197)
     * ```
     */
    vec2& operator+=(const vec2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
};