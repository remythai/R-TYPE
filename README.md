# 🛸 R-Type — Online Multiplayer Game

[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://remythai.github.io/R-TYPE/)

## Project Overview

**R-Type** is a multiplayer shoot 'em up game developed as part of the **Advanced C++ / Network Programming** module at **Epitech Technology**.

This project recreates the classic R-Type experience with modern multiplayer capabilities, implementing:
- **Client/server architecture** in **C++17**
- **UDP network communication** via **Asio** for low-latency real-time gameplay
- **Modular game engine** based on **ECS (Entity Component System)**
- Support for **up to 4 simultaneous players**

Players can connect to a server, choose a game, control their character, shoot enemies or avoid obstacles, and face waves of opponents or obstacles in real-time cooperative gameplay.

---

## 🚀 Quick Start

### Prerequisites
- **CMake ≥ 3.16**
- **C++17 compiler** (GCC, Clang, or MSVC)
- **Asio** (standalone or Boost.Asio)
- **SFML 3.0.2** (for client rendering)

### Supported Platforms
- Linux
- Windows

### Build & Run

#### 1. Clone the repository
```bash
git clone https://github.com/EpitechPGE3-2025/G-CPP-500-BDX-5-1-rtype-7.git
cd G-CPP-500-BDX-5-1-rtype-7
```

#### 2. Build the project
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Executables will be generated at the project root:
- `r-type_server` — Game server
- `r-type_client` — Game client

#### 3. Run the server
```bash
./r-type_server -h 0.0.0.0 -p 8080
```

#### 4. Run the client(s)
```bash
./r-type_client -h 127.0.0.1 -p 8080
```

Multiple clients can connect to the same server for multiplayer gameplay.

---

## 📁 Project Structure

```
R-Type/
├── .github/              # CI/CD workflows
├── assets/               # Game sprites and resources
├── client/               # Client application
│   ├── src/
│   └── CMakeLists.txt
├── gameEngine/           # ECS game engine
│   ├── src/
│   ├── include/
│   └── docs/
├── server/               # Server application
│   ├── src/
│   ├── docs/
│   └── CMakeLists.txt
├── CMakeLists.txt        # Root build configuration
└── README.md
```

---

## 🎮 Key Features

### Client Features
- Real-time server connection via UDP
- Keyboard input handling (movement, shooting)
- Entity rendering (player ships, enemies, projectiles, power-ups)
- Network synchronization with server snapshots
- Low-latency gameplay experience

### Server Features
- Asynchronous UDP networking with **Asio**
- Dynamic player ID assignment
- Player slot management (max 4 players)
- Game event broadcasting to all connected clients
- Entity management: spawn, movement, collision, destruction
- Timeout-based disconnection detection

### Technical Architecture
- **ECS (Entity Component System)** for modular and scalable game logic
- **UDP protocol** for minimal latency (see [comparative study](server/docs/study-comparaison.md))
- **Binary packet serialization** for efficient network transmission
- **Timestamp synchronization** for consistent game state

---

## 🔌 Network Protocol

The game uses **UDP** with a compact binary packet structure:

```
[Type:1 byte][PacketID:2 bytes][Timestamp:4 bytes][Payload:variable]
```

### Example Packets

**Client → Server:**
| Type   | Name          | Description                          |
|--------|---------------|--------------------------------------|
| `0x01` | INPUT         | Player movement and shooting input   |
| `0x02` | JOIN          | Connection request with username     |
| `0x03` | PING          | Latency measurement                  |
| `0x04` | DISCONNECT    | Player leaving notification          |

**Server → Client:**
| Type   | Name                 | Description                          |
|--------|----------------------|--------------------------------------|
| `0x08` | PLAYER_ID_ASSIGNMENT | Unique player ID assignment          |
| `0x09` | PLAYER_LIST          | List of connected players            |
| `0x10` | SNAPSHOT             | Full game state synchronization      |
| `0x11` | ENTITY_EVENT         | Entity spawn/destroy events          |
| `0x12` | PLAYER_EVENT         | Player-specific events (death, score)|

For complete protocol specification, see [Network Protocol Documentation](server/docs/protocol.md).

---

## 📚 Documentation

- **[API Documentation](https://remythai.github.io/R-TYPE/)** — Doxygen-generated API reference
- **[Game Engine Guide](https://remythai.github.io/R-TYPE/gameEngine)** — How to implement game logic with our ECS
- **Comparative Studies:**
  - [Why ECS for Game Engine?](gameEngine/docs/Why_ecs.md)
  - [Why SFML for Rendering?](client/docs/Why_SFML.md)
  - [Why UDP over TCP?](server/docs/study-comparaison.md)

---

## 🧪 Testing

Unit tests can be enabled during build:

```bash
cmake -DENABLE_TESTS=ON ..
cmake --build .- **[Game Engine Guide](https://remythai.github.io/R-TYPE/gameEngine)** — How to implement game logic with our ECS
ctest
```

Tests cover:
- Packet serialization/deserialization
- ECS component behavior
- Client/server interaction logic

---

## 👥 Development Team

| Name                  | Role                              |
|-----------------------|-----------------------------------|
| **Antton Ducos**      | ECS Developer / Backend           |
| **Louka Ortega-cand** | ECS Developer / Game Logic        |
| **Rémy Thai**         | Client Developer / Game Interface |
| **Simon Maigrot**     | Network Developer / UDP Server    |

---

## 📖 Additional Resources

- **Asio Documentation**: https://think-async.com
- **ECS Pattern**: https://skypjack.github.io/entt/
- **UDP Game Networking**: [Valve Developer Wiki](https://developer.valvesoftware.com/wiki/Source_Multiplayer_Networking)
- **SFML Documentation**: https://www.sfml-dev.org

---

## ⚖️ License

This project was developed as part of Epitech's educational curriculum.  
Use is restricted to learning and technical demonstration purposes.

---

## 📞 Contact

For questions, suggestions, or contributions, please open an issue on the GitHub repository.

**Repository**: https://github.com/EpitechPGE3-2025/G-CPP-500-BDX-5-1-rtype-7