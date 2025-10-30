# 🛸 R-Type — Online Multiplayer Game (C++ / Asio / ECS)

## Project Overview

**R-Type** is a project developed as part of the **Advanced C++ / Network Programming** module at **Epitech Technology**.
The goal is to **recreate a multiplayer game inspired by the classic R-Type**, implementing:
- A **client/server architecture** in **C++17** (or higher)
- **UDP** network communication (via **Asio**)
- A **modular game engine** based on an **ECS (Entity Component System)**

The game allows multiple players to connect, move, shoot, and face waves of enemies in real-time.

---

## dependancies
- **CMake ≥ 3.20**
- **C++17** (or higher)
- **Asio**
- **SFML** (depending on your rendering engine)
- **Conan** (optional, for library management)

## supported plateforms
- Linux
- Windows

## Main Features

### Player Side (Client)
- Server connection (JOIN)
- Keyboard input management
- Real-time action sending via UDP
- Game world and entity display (ships, shots, enemies)
- Position synchronization via network snapshots

### Server Side
- Asio network loop management (async)
- Client packet reception and processing
- Dynamic PlayerID assignment
- Game event broadcasting to all clients
- Player slot system (max. 4 players)
- Event management: spawn, movement, shooting, collision, death

### Internal Architecture
- **ECS (Entity Component System)** for modular gameplay management
- **Main asynchronous network thread** (Asio)
- **Binary serialization system** for network packets
- **Timestamp and packetId management** for synchronization

---

## Project Structure

```
R-Type/
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── architecture.md
│   ├── protocol.md
│   ├── comparative-study.md
│   └── accessibility.md
├── src/
│   ├── client/
│   │   ├── main.cpp
│   │   └── ...
│   ├── server/
│   │   ├── NetworkServer.cpp
│   │   └── ...
│   └── engine/
│       ├── ecs/
│       └── components/
├── assets/
│   └── sprites/
└── tests/
```

---

## ⚙️ Installation and Compilation

### Dependencies
Make sure you have:
- **CMake ≥ 3.20**
- **C++17** (or higher)
- **Asio** (or Boost.Asio)
- **SFML** (3.0.2)

### Compilation

#### 1. Clone the project:
```bash
git clone https://github.com/EpitechPGE3-2025/G-CPP-500-BDX-5-1-rtype-7.git
cd G-CPP-500-BDX-5-1-rtype-7
```

#### 2. Compile and run the server:
```bash
cd server
./build.sh
./r-type_server -h [hostname] -p [port]
```

#### 3. Compile and run the client:
```bash
cd client
./build.sh
./r-type_client -h [hostname] -p [port]
```

---

## 🔌 Network Communication

The protocol is based on UDP with a fixed binary structure:

```
[Type:1][PacketID:2][Timestamp:4][Payload:n]
```

Packet examples:

Client to Server:
| **Type (hex)** | **Name**      | **Payload Structure**                             | **Description**                          |
| -------------- | ------------- | ------------------------------------------------- | ---------------------------------------- |
| `0x01`         | INPUT         | `[PlayerID:1][InputMask:1]`                       | Player keyboard input (movement, shoot). |
| `0x02`         | JOIN          | `[UsernameLength:1][Username:variable]`           | Request to join a game.                  |
| `0x03`         | PING          | `[Timestamp:4]`                                   | Latency test.                            |
| `0x04`         | DISCONNECT    | `[PlayerID:1]`                                    | Player leaving game.                     |
| `0x14`         | SPAWN_REQUEST | `[EntityType:1][X:4][Y:4]`                        | Request to spawn entity (e.g., bullet).  |
| `0x1B`         | CHAT_MESSAGE  | `[PlayerID:1][MessageLength:1][Message:variable]` | Send in-game chat message.               |

Server to Client:
| **Type (hex)** | **Name**             | **Payload Structure**                                                                                    | **Description**                                           |
| -------------- | -------------------- | -------------------------------------------------------------------------------------------------------- | --------------------------------------------------------- |
| `0x08`         | PLAYER_ID_ASSIGNMENT | `[PlayerID:1]`                                                                                           | Assign unique player ID.                                  |
| `0x09`         | PLAYER_LIST          | `[Count:1][Players:(ID:1)(X:4)(Y:4)]×Count`                                                              | List of players and positions.                            |
| `0x10`         | SNAPSHOT             | `[EntityCount:2][Entities:[EntityID:2][Type:1][X:4][Y:4][Rotation:2][HP:1][Extra:variable]]×EntityCount` | Full world sync (players, enemies, bullets, power-ups).   |
| `0x11`         | ENTITY_EVENT         | `[EventType:1][EntityID:2][EntityType:1][X:4][Y:4]`                                                      | Entity spawn, shoot, destroy, collision.                  |
| `0x12`         | PLAYER_EVENT         | `[PlayerID:1][EventType:1][Value:4]`                                                                     | Player-specific events (death, respawn, score, power-up). |
| `0x13`         | PING_RESPONSE        | `[Timestamp:4]`                                                                                          | Ping reply for RTT calculation.                           |
| `0x15`         | SPAWN_CONFIRMATION   | `[EntityID:2][EntityType:1][X:4][Y:4]`                                                                   | Confirms entity creation.                                 |
| `0x16`         | HIT_EVENT            | `[EntityHit:2][EntitySource:2][Damage:1]`                                                                | Entity took damage.                                       |
| `0x17`         | ENTITY_REMOVE        | `[EntityID:2]`                                                                                           | Remove entity from world.                                 |
| `0x18`         | GAME_STATE           | `[State:1]`                                                                                              | Global game state (Lobby, InGame, GameOver, Victory).     |
| `0x19`         | LEVEL_EVENT          | `[LevelID:1][EventType:1]`                                                                               | Level-specific events (boss, wave end).                   |
| `0x1A`         | SCORE_UPDATE         | `[PlayerID:1][Score:4]`                                                                                  | Update player score.                                      |
| `0x1C`         | POWERUP_COLLECTED    | `[PlayerID:1][PowerUpType:1]`                                                                            | Player picked up a power-up.                              |
| `0x1D`         | RESPAWN_PLAYER       | `[PlayerID:1][X:4][Y:4]`                                                                                 | Respawn dead player.                                      |
| `0x1E`         | SERVER_SHUTDOWN      | —                                                                                                        | Server is shutting down.                                  |

For detailed protocol documentation, see [docs/protocol.md](docs/protocol.md)

---

## 🧩 Technical Architecture

### 🔹 Server Side
- **NetworkServer**: Handles packet reception and transmission
- **PlayerSlot**: Connected player structure
- **PacketHandler**: Processing logic (JOIN, INPUT, etc.)
- **GameWorld**: Game logic and synchronization

### 🔹 Client Side
- **NetworkClient**: Handles server communication
- **Renderer**: Game display
- **InputManager**: Keyboard event reading
- **ECS**: Entity and component management

---

## 👥 Team

| Name                      | Role                              |
|---------------------------|-----------------------------------|
| **Antton Ducos**          | ECS Developer / Backend           |
| **Louka Ortega-cand**     | ECS Developer / Game Logic        |
| **Rémy Thai**             | Client Developer / Game Interface |
| **Simon Maigrot**         | Network Developer / UDP Server    |

---

## 🧪 Testing

Unit tests can be run with:
```bash
cd build
ctest
```

They cover:
- Packet serialization
- Client/server interactions
- ECS components

---

## 🧭 Technical Resources

- **Asio Documentation**: https://think-async.com
- **ECS Pattern**: https://skypjack.github.io/entt/
- **UDP Game Networking**: Valve Developer Wiki
- **SFML doc**: https://www.sfml-dev.org /

---

## 📚 Documentation

- [![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://remythai.github.io/R-TYPE/)
- [Network Protocol Specification](docs/protocol.md)
- [Server Architecture](docs/server-architecture.md)
- [ECS Engine Design](https://remythai.github.io/R-TYPE/gameEngine)
- [Comparative Technical Study](docs/comparative-study.md)
- [Accessibility Guidelines](docs/accessibility.md)

---

## ⚖️ License

Project developed as part of Epitech's educational curriculum.
Use restricted to learning and technical demonstration purposes.

---

## 📞 Contact

For questions or contributions, please open an issue on the GitHub repository or contact the development team.

**Project Repository**: https://github.com/EpitechPGE3-2025/G-CPP-500-BDX-5-1-rtype-7
