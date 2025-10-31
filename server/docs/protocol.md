# R-Type Network Protocol Documentation

## Overview

This document describes the binary communication protocol used between the R-Type client and server. The protocol operates over UDP and follows a structured packet format for efficient, low-latency communication.

## Protocol Characteristics

- **Transport Layer**: UDP (User Datagram Protocol)
- **Encoding**: Binary format with big-endian byte order
- **Connection Model**: Connectionless with client identification via Player ID assignment
- **Maximum Players**: 4 concurrent players per game instance
- **State Synchronization**: Snapshot-based (all game state and events are transmitted via periodic snapshots)

## Packet Structure

All packets follow a common header structure followed by a variable-length payload:

```
┌─────────────┬─────────────┬─────────────┬─────────────────────┐
│   Header    │  Packet ID  │  Timestamp  │       Payload       │
│   (1 byte)  │  (2 bytes)  │  (4 bytes)  │    (Variable)       │
└─────────────┴─────────────┴─────────────┴─────────────────────┘
    Type         Sequence      Time Info      Type-specific data
```

### Header Fields

| Field | Size | Type | Description |
|-------|------|------|-------------|
| **Packet Type** | 1 byte | uint8_t | Identifies the packet type (see PacketType enum) |
| **Packet ID** | 2 bytes | uint16_t | Sequence number for packet ordering |
| **Timestamp** | 4 bytes | uint32_t | Timestamp for temporal ordering |
| **Payload** | Variable | uint8_t[] | Packet-specific data |

**Total Header Size**: 7 bytes

### Byte Order

All multi-byte integers use **big-endian** (network byte order) encoding:
- uint16_t: Most significant byte first
- uint32_t: Most significant byte first
- float: IEEE 754 format, big-endian byte order

## Packet Types

### 0x01 - INPUT

Player input commands sent from client to server.

**Direction**: Client → Server

**Payload Structure**:
```
┌─────────────┬─────────────┬─────────────┐
│  Player ID  │   Key Code  │   Action    │
│   (1 byte)  │   (1 byte)  │   (1 byte)  │
└─────────────┴─────────────┴─────────────┘
```

| Field | Size | Description |
|-------|------|-------------|
| Player ID | 1 byte | ID of the player sending input (0-3) |
| Key Code | 1 byte | Key identifier (e.g., arrow keys, spacebar) |
| Action | 1 byte | Action type: 0=Key Up, 1=Key Down |

**Example**: Player 0 presses the up arrow key
```
01 00 00 00 00 00 00 00 26 01
│  │     │  │        │  │  │
│  │     │  │        │  │  └─ Action (1 = pressed)
│  │     │  │        │  └──── Key code (0x26 = Up arrow)
│  │     │  │        └─────── Player ID (0)
│  │     │  └──────────────── Timestamp
│  │     └─────────────────── Packet ID
│  └───────────────────────── Type (INPUT)
└──────────────────────────── Header
```

---

### 0x02 - JOIN

Connection request from client with player identification.

**Direction**: Client → Server

**Payload Structure**:
```
┌──────────────────────────┐
│       Username           │
│    (Variable length)     │
└──────────────────────────┘
```

| Field | Size | Description |
|-------|------|-------------|
| Username | Variable | UTF-8 encoded player username (null-terminated or length-prefixed) |

**Server Response**: Upon receiving a JOIN packet, the server assigns a Player ID and responds with a `PLAYER_ID_ASSIGNMENT` packet.

**Example**: Client joins with username "Player1"
```
02 00 00 00 00 00 00 50 6C 61 79 65 72 31
│  │     │  │        │
│  │     │  │        └─────── Username bytes (ASCII "Player1")
│  │     │  └──────────────── Timestamp
│  │     └─────────────────── Packet ID
│  └───────────────────────── Type (JOIN)
└──────────────────────────── Header
```

---

### 0x08 - PLAYER_ID_ASSIGNMENT

Server assigns a Player ID to a newly connected client.

**Direction**: Server → Client

**Payload Structure**:
```
┌─────────────┐
│  Player ID  │
│   (1 byte)  │
└─────────────┘
```

| Field | Size | Description |
|-------|------|-------------|
| Player ID | 1 byte | Assigned player identifier (0-3, or 255 if full) |

**Player ID Values**:
- `0-3`: Valid player slot
- `255`: No available slots (server full)

---

### 0x10 - SNAPSHOT

Game state update from server to clients. This packet contains the complete state of all visible entities in the game.

**Direction**: Server → Client

**Payload Structure**:
```
┌──────────────┬─────────────────────────────────┐
│              │   Entity Data (repeated per    │
│              │        entity with Renderable)  │
└──────────────┴─────────────────────────────────┘

Per Entity (repeated):
┌─────────────┬─────────────┬─────────────┬─────────────┬──────────────┬─────────────┬─────────────┬─────────────┬─────────────┐
│  Entity ID  │  Position X │  Position Y │ Path Length │ Sprite Path  │  Rect Pos X │  Rect Pos Y │ Rect Size X │ Rect Size Y │
│   (1 byte)  │  (4 bytes)  │  (4 bytes)  │  (1 byte)   │  (Variable)  │  (4 bytes)  │  (4 bytes)  │  (4 bytes)  │  (4 bytes)  │
└─────────────┴─────────────┴─────────────┴─────────────┴──────────────┴─────────────┴─────────────┴─────────────┴─────────────┘
```

| Field | Size | Description |
|-------|------|-------------|
| Entity ID | 1 byte | Unique identifier of the entity |
| Position X | 4 bytes | X coordinate (float, big-endian) |
| Position Y | 4 bytes | Y coordinate (float, big-endian) |
| Path Length | 1 byte | Length of sprite sheet path string |
| Sprite Path | Variable | UTF-8 encoded path to sprite sheet |
| Rect Pos X | 4 bytes | Current sprite rectangle X position (float) |
| Rect Pos Y | 4 bytes | Current sprite rectangle Y position (float) |
| Rect Size X | 4 bytes | Sprite rectangle width (float) |
| Rect Size Y | 4 bytes | Sprite rectangle height (float) |

**Note**: The snapshot includes all entities that have both a `Renderable` and `Position` component. Entities without these components are not transmitted. The client should update or create entities based on the received data and remove entities not present in the snapshot.

**Broadcast Frequency**: Snapshots are sent at a rate defined by `SNAPSHOT_RATE` (typically ~60 Hz or every 16ms).

---

### 0x20 - TIMEOUT

Player disconnection notification sent from server to all clients when a player becomes inactive.
Direction: Server → Client (Broadcast)
Payload Structure:
┌─────────────┬─────────────┬──────────────┬──────────────┐
│  Entity ID  │  Player ID  │ Username Len │   Username   │
│   (1 byte)  │   (1 byte)  │   (1 byte)   │  (Variable)  │
└─────────────┴─────────────┴──────────────┴──────────────┘
FieldSizeDescriptionEntity ID1 byteID of the destroyed entity (0-255)Player ID1 byteID of the disconnected player (0-3)Username Length1 byteLength of the username string (0-255)UsernameVariableUTF-8 encoded player username
Trigger Conditions:

Player inactive for 30+ seconds (no packets received)
Manual disconnection without proper cleanup

Example: Player 2 ("Alice") times out with entity ID 5
20 00 00 00 00 00 00 05 02 05 41 6C 69 63 65
│  │     │  │        │  │  │  │
│  │     │  │        │  │  │  └─────── Username bytes (ASCII "Alice")
│  │     │  │        │  │  └────────── Username length (5)
│  │     │  │        │  └───────────── Player ID (2)
│  │     │  │        └──────────────── Entity ID (5)
│  │     │  └───────────────────────── Timestamp
│  │     └──────────────────────────── Packet ID
│  └────────────────────────────────── Type (TIMEOUT = 0x20)
└───────────────────────────────────── Header
**Server Behavior:**

Detect player inactivity (30+ seconds without packets)
Destroy the player's entity in the ECS registry
Free the player slot for reuse
Construct and broadcast TIMEOUT packet to all remaining clients
Log: "Player <ID> (<username>) timed out. Entity: <entityID>"

**Client Behavior:**
Upon receiving a TIMEOUT packet, clients should:

Parse the entity ID, player ID, and username from payload
Remove the specified entity from the local game world
Display notification: "Player <username> timed out"
Update UI/HUD to reflect player count
If local player ID matches: initiate graceful client shutdown

---

### 0x20 - KILLED

Player elimination notification sent from server to all clients when a player entity is destroyed due to in-game death.
Direction: Server → Client (Broadcast)
Payload Structure:

┌─────────────┬─────────────┬──────────────┬──────────────┐
│  Entity ID  │  Player ID  │ Username Len │   Username   │
│   (1 byte)  │   (1 byte)  │   (1 byte)   │  (Variable)  │
└─────────────┴─────────────┴──────────────┴──────────────┘

FieldSizeDescriptionEntity ID1 byteID of the destroyed entity (0-255)Player ID1 byteID of the eliminated player (0-3)Username Length1 byteLength of the username string (0-255)UsernameVariableUTF-8 encoded player username
Trigger Conditions:

Player entity Health component reaches 0
Death system (GameEngine::Death) processes entity destruction
Manual entity elimination (e.g., out-of-bounds)

Example: Player 1 ("Bob") is killed with entity ID 3
40 00 00 00 00 00 00 03 01 03 42 6F 62
│  │     │  │        │  │  │  │
│  │     │  │        │  │  │  └─────── Username bytes (ASCII "Bob")
│  │     │  │        │  │  └────────── Username length (3)
│  │     │  │        │  └───────────── Player ID (1)
│  │     │  │        └──────────────── Entity ID (3)
│  │     │  └───────────────────────── Timestamp
│  │     └──────────────────────────── Packet ID
│  └────────────────────────────────── Type (KILLED = 0x40)
└───────────────────────────────────── Header
**Server Behavior**:

Detect entity death (Health <= 0 or Death component trigger)
Destroy the player's entity in the ECS registry
Mark player slot as unused (can respawn or rejoin)
Construct and broadcast KILLED packet to all clients
Log: "Player <ID> (<username>) eliminated. Entity: <entityID>"

**Client Behavior**:
Upon receiving a KILLED packet, clients should:

Parse the entity ID, player ID, and username from payload
Play death animation/effect for the entity (if applicable)
Remove the specified entity from the local game world after animation
Display notification: "Player <username> was eliminated"
Update kill feed/scoreboard
If local player ID matches: show "You were eliminated" screen
Optional: Spectator mode or respawn countdown

## Connection Flow

### Initial Connection Sequence

```
Client                                Server
  |                                     |
  |  (1) JOIN (username)                |
  |------------------------------------>|
  |                                     |
  |                          (2) Assign Player ID
  |                          (3) Create Player Entity
  |                                     |
  |  (4) PLAYER_ID_ASSIGNMENT           |
  |<------------------------------------|
  |                                     |
  |  Ready to play                      |
  |<===================================>|
```

### Game Session

```
Client                                Server
  |                                     |
  |  INPUT packets (continuous)         |
  |------------------------------------>|
  |                                     |
  |  SNAPSHOT (game state, ~60Hz)       |
  |<------------------------------------|
  |                                     |
  |  SNAPSHOT (game state, ~60Hz)       |
  |<------------------------------------|
  |                                     |
```

**Key Points**:
- All game events (player joins, deaths, spawns, collisions) are implicitly communicated through snapshots
- Clients detect events by comparing snapshot differences
- No explicit event packets are needed

---

## Error Handling

### Malformed Packets

- **Minimum packet size**: 7 bytes (header only)
- Packets smaller than 7 bytes are silently discarded
- Packets with invalid type codes are logged and ignored
- The server **MUST NOT** crash on malformed packets

### Player ID Validation

When the server receives an INPUT packet, it validates that:
1. The Player ID in the payload matches the endpoint's assigned Player ID
2. If mismatched, a warning is logged but the packet is not rejected

### Duplicate JOIN Requests

If a client sends multiple JOIN packets:
- The server recognizes the existing connection via endpoint matching
- Server resends the `PLAYER_ID_ASSIGNMENT` with the already-assigned Player ID
- No new player slot is allocated

### Server Full

When all 4 player slots are occupied:
- Server responds with `PLAYER_ID_ASSIGNMENT` containing Player ID = `255`
- Client should display "Server Full" message and disconnect

### Client Timeout

- Clients that don't send packets for 30 seconds are considered inactive
- Server automatically cleans up inactive player slots
- Player entities are destroyed and slots are freed for new players

---

## Security Considerations

### Buffer Overflow Prevention

- All incoming packets are read into fixed-size buffers (1024 bytes)
- Payload size is calculated as: `bytesReceived - 7`
- No unbounded memory allocation based on packet data

### Packet Injection

- Player ID validation ensures clients cannot impersonate other players
- Server maintains authoritative player slot mapping by endpoint
- Future implementations should include:
  - Session tokens
  - Packet signing/authentication
  - Rate limiting

### Denial of Service

Current mitigations:
- Fixed maximum packet size
- Limited player slots (4 maximum)
- Automatic cleanup of inactive connections

Recommended additions:
- Rate limiting on JOIN requests per IP
- Packet flood detection
- Bandwidth throttling

---

## Implementation Notes

### Serialization Helpers

The implementation provides template functions for byte conversion:

```cpp
// Convert value to big-endian byte array
template<typename T>
std::vector<uint8_t> toBytes(T value);

// Convert big-endian byte array to value
template<typename T>
T fromBytes(const uint8_t* data);

// Convert float to big-endian byte array
std::vector<uint8_t> floatToBytes(float value);
```

### Thread Safety

- Client endpoint map (`_clients`) is protected by mutex (`_clientsMutex`)
- Player slots array (`_playerSlots`) access is synchronized with `_playerSlotsMutex`
- Registry access is protected by `_registryMutex`
- ASIO's `async_receive_from` ensures safe concurrent packet handling

### ECS Integration

- Server runs ECS update loop at 120 Hz
- Snapshots are broadcast at ~60 Hz (every 16ms)
- Entity spawning (enemies) occurs every 5 seconds
- Input commands are immediately applied to player entities via `InputControlled` component

### UDP Reliability Considerations

Since UDP is unreliable:
- Critical messages (JOIN, PLAYER_ID_ASSIGNMENT) may need retransmission
- Client should retry JOIN if no response within timeout
- Snapshot-based approach provides natural state recovery (missed packets are overwritten by next snapshot)

---

## Future Extensions

Potential protocol enhancements:

1. **Packet Compression**: Add compression flag in header, compress payload with LZ4/zlib
2. **Delta Compression**: Send only changed entity data in SNAPSHOT (compare with previous snapshot)
3. **Message Fragmentation**: Support for packets > 1024 bytes
4. **Encryption**: Add optional payload encryption for sensitive data
5. **Lobby System**: Packets for game room creation, discovery, matchmaking
6. **Interpolation Data**: Include velocity/acceleration in snapshots for smoother client-side prediction
7. **Acknowledged Messages**: Optional reliability layer for critical non-snapshot packets

---

## Appendix: Packet Type Reference Table

| Hex | Dec | Name | Direction | Description |
|-----|-----|------|-----------|-------------|
| 0x01 | 1 | INPUT | C→S | Player input command |
| 0x02 | 2 | JOIN | C→S | Connection request |
| 0x08 | 8 | PLAYER_ID_ASSIGNMENT | S→C | Player ID assignment |
| 0x10 | 16 | SNAPSHOT | S→C | Complete game state update |

**Legend**: C = Client, S = Server, → = Unidirectional

---

## Contact & Contribution

For questions, bug reports, or protocol enhancement proposals, please refer to the project's developer documentation and contribution guidelines.

**Protocol Version**: 2.0
**Last Updated**: October 2025
**Compatibility**: R-Type Server v2.x
**Author**: Simon Maigrot
**Contact**: simon.maigrot@epitech.eu