# 🛸 R-Type — Online Multiplayer Game (C++ / Asio / ECS)

## Project Overview

**R-Type** is a project developed as part of the **Advanced C++ / Network Programming** module at **Epitech Technology**.  
The goal is to **recreate a multiplayer game inspired by the classic R-Type**, implementing:
- A **client/server architecture** in **C++17** (or higher)
- **UDP** network communication (via **Asio**)
- A **modular game engine** based on an **ECS (Entity Component System)**

The game allows multiple players to connect, move, shoot, and face waves of enemies in real-time.

---

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
- **SFML** or **Raylib** (depending on your rendering engine)
- **Conan** or **Vcpkg** (optional, for library management)

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

| Type | Name                 | Description                    |
|------|----------------------|--------------------------------|
| 0x01 | INPUT                | Keyboard action sent           |
| 0x02 | JOIN                 | A player joins the game        |
| 0x03 | PING                 | Latency check                  |
| 0x08 | PLAYER_ID_ASSIGNMENT | Unique ID assignment to player |
| 0x10 | SNAPSHOT             | World synchronization          |
| 0x11 | ENTITY_EVENT         | Entity-related events          |
| 0x12 | PLAYER_EVENT         | Player-related events          |
| 0x13 | PING_RESPONSE        | Response to ping request       |

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
- **SFML / Raylib Docs**: https://www.sfml-dev.org / https://www.raylib.com

---

## 📚 Documentation

- [Network Protocol Specification](docs/protocol.md)
- [Server Architecture](docs/server-architecture.md)
- [ECS Engine Design](docs/architecture.md)
- [Comparative Technical Study](docs/comparative-study.md)
- [Accessibility Guidelines](docs/accessibility.md)

---

## 🚀 Roadmap

### Part 1 (Prototype - Week 4)
- [x] Basic UDP server with player management
- [x] Binary protocol implementation
- [x] Client connection and input handling
- [ ] Game world rendering
- [ ] Basic enemy spawning
- [ ] Shooting mechanics

### Part 2 (Advanced Features - Week 7)
- [ ] Multi-instance server (multiple game rooms)
- [ ] Lobby system
- [ ] Advanced networking (compression, reliability)
- [ ] Complete gameplay (bosses, levels, weapons)
- [ ] Level editor
- [ ] Performance optimization

---

## ⚖️ License

Project developed as part of Epitech's educational curriculum.  
Use restricted to learning and technical demonstration purposes.

---

## 📞 Contact

For questions or contributions, please open an issue on the GitHub repository or contact the development team.

**Project Repository**: https://github.com/EpitechPGE3-2025/G-CPP-500-BDX-5-1-rtype-7