# 🎮 GameEngine Components Library

A modular **C++ component library** for a custom **Game Engine**, providing reusable ECS-style data structures such as position, velocity, collider, AI control, and more.  
This library is designed to be **compiled as a shared library** using **CMake**, making it easy to integrate into any modern game engine or simulation project.

---

## 📖 How to Use

1. Include the desired components in your game project:
   ```cpp
   #include "GameEngine/components/Position.hpp"
   #include "GameEngine/components/Velocity.hpp"
   ```

2. Create entities and attach components:
   ```cpp
   GameEngine::Position pos(10.0f, 5.0f);
   GameEngine::Velocity vel(2.0f, 0.0f);
   ```

3. Use them in your ECS system or logic loop:

4. Compile and link your project against the shared library built via CMake.

---

## 💻 Installation & Setup

1. **Clone the repository**:
   ```sh
   git clone https://github.com/your-username/GameEngineComponents.git
   cd GameEngineComponents
   ```

2. **Build the shared library using CMake**:
   ```sh
   mkdir build && cd build
   cmake ..
   make
   ```

3. **Link the library** in your main project’s `CMakeLists.txt`:
   ```cmake
   target_link_libraries(MyGame PRIVATE GameEngineComponents)
   ```

4. **Include the headers** in your code:
   ```cpp
   #include <GameEngine/components/Health.hpp>
   #include <GameEngine/components/Weapon.hpp>
   ```

---

## 🎮 Features

```
✔ Modular ECS-compatible component definitions  
✔ Physics-based attributes (Position, Velocity, Acceleration)  
✔ Combat-ready structures (Health, Damage, Weapon)  
✔ Interaction & event systems (OnPickup, DropOnDeath)  
✔ AI and player control markers (AIControlled, InputControlled)  
✔ Collision & hitbox mechanics (Collider, HitBox)  
✔ Audio and text rendering support  
✔ Fully CMake-compatible shared library build
```

---

## 🔧 Project Structure

```
/GameEngineComponents
│── CMakeLists.txt
│── /GameEngine
│   ├── Acceleration.hpp
│   ├── Position.hpp
│   ├── Velocity.hpp
│   ├── Health.hpp
│   ├── Damage.hpp
│   ├── Collider.hpp
│   ├── HitBox.hpp
│   ├── AIControlled.hpp
│   ├── InputControlled.hpp
│   ├── Weapon.hpp
│   ├── Audio.hpp
│   ├── Text.hpp
│   ├── OnPickup.hpp
│   ├── DropOnDeath.hpp
│   ├── ScoreValue.hpp
│   ├── Lifetime.hpp
│── /utils.hpp
│── README.md
```

---

## 📌 Example Usage

```cpp
#include "GameEngine/components/Position.hpp"
#include "GameEngine/components/Velocity.hpp"
#include "GameEngine/components/Health.hpp"
#include <iostream>

int main() {
    GameEngine::Position pos(0.0f, 0.0f);
    GameEngine::Velocity vel(1.0f, 0.5f);
    GameEngine::Health hp(100, 100);

    float deltaTime = 0.016f; // ~60 FPS
    pos.x += vel.x * deltaTime;
    pos.y += vel.y * deltaTime;

    std::cout << "Position: (" << pos.x << ", " << pos.y << ")\n";
    std::cout << "Health: " << hp.currentHp << "/" << hp.maxHp << std::endl;

    return 0;
}
```

---

## 🛠 Adding a New Component

```
1. Create a new header file in `GameEngine/components/`:
   ```cpp
   #pragma once
   namespace GameEngine {
   struct Mana {
       int current;
       int max;
       Mana(int val_current = 0, int val_max = 100)
           : current(val_current), max(val_max) {}
   };
   }
   ```

2. Include it in your ECS or logic systems as needed.

3. The build system (CMake) automatically detects header-only components.
```

---

## 🛠 Future Improvements

```
- Add more physics and rendering components  
- Introduce serialization for saving/loading entity states  
- Create example ECS system integration  
- Improve documentation and API references  
```

---

## 📜 License

```
This project is open-source under the **MIT License**.
```

---

## 💡 Credits

```
- Created by [Your Name]  
- Built with C++ and CMake 🧩  
- Designed for modern ECS-based Game Engines  
```

---

### 🚀 Enjoy Building Your Game! 🎉
