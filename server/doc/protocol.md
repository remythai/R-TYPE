# R-Type Network Protocol Documentation

## Overview

This document describes the binary communication protocol used between the R-Type client and server. The protocol operates over UDP and follows a structured packet format for efficient, low-latency communication.

## Protocol Characteristics

- **Transport Layer**: UDP (User Datagram Protocol)
- **Encoding**: Binary format with big-endian byte order
- **Connection Model**: Connectionless with client identification via Player ID assignment
- **Maximum Players**: 4 concurrent players per game instance

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
| **Packet ID** | 2 bytes | uint16_t | Sequence number for packet ordering and acknowledgment |
| **Timestamp** | 4 bytes | uint32_t | Timestamp for latency calculation and temporal ordering |
| **Payload** | Variable | uint8_t[] | Packet-specific data |

**Total Header Size**: 7 bytes

### Byte Order

All multi-byte integers use **big-endian** (network byte order) encoding:
- uint16_t: Most significant byte first
- uint32_t: Most significant byte first

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
| Action | 1 byte | Action type: 0=Key Down, 1=Key Up |

**Example**: Player 0 presses the up arrow key
```
01 00 00 00 00 00 00 00 26 00
│  │     │  │        │  │  │
│  │     │  │        │  │  └─ Action (0 = pressed)
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
| Username | Variable | UTF-8 encoded player username (null-terminated) |

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

### 0x03 - PING

Latency measurement request.

**Direction**: Client → Server

**Payload**: Empty (0 bytes)

**Server Response**: Server immediately responds with a `PING_RESPONSE` packet containing the same Packet ID and Timestamp for RTT calculation.

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

Game state update from server to clients.

**Direction**: Server → Client

**Payload Structure**:
```
┌──────────────┬─────────────────────────────────┐
│  Nb Entities │      Entity Data (repeated)     │
│   (1 byte)   │         (Variable)              │
└──────────────┴─────────────────────────────────┘
```

| Field | Size | Description |
|-------|------|-------------|
| Entity Count | 1 byte | Number of entities in this snapshot |
| Entity Data | Variable | Serialized entity information (format TBD) |

**Note**: The specific encoding of Entity Data depends on your game engine's entity serialization format. Typically includes: entity ID, position (x, y), velocity, sprite/type, health, etc.

---

### 0x11 - ENTITY_EVENT

Notification of entity-specific events (spawn, destruction, collision, etc.).

**Direction**: Bidirectional (primarily Server → Client)

**Payload Structure**:
```
┌─────────────┬─────────────┬─────────────────┐
│  Entity ID  │  Event Type │   Extra Data    │
│   (1 byte)  │   (1 byte)  │   (Variable)    │
└─────────────┴─────────────┴─────────────────┘
```

| Field | Size | Description |
|-------|------|-------------|
| Entity ID | 1 byte | Identifier of the entity |
| Event Type | 1 byte | Type of event (see Entity Event Types) |
| Extra Data | Variable | Event-specific additional data |

**Entity Event Types** (examples):
- `0x00`: Entity spawned
- `0x01`: Entity destroyed
- `0x02`: Entity damaged
- `0x03`: Entity fired weapon
- `0x04`: Entity collision

---

### 0x12 - PLAYER_EVENT

Notification of player-specific events (join, leave, score update, death, etc.).

**Direction**: Server → Client

**Payload Structure**:
```
┌─────────────┬─────────────┬─────────────┐
│  Player ID  │  Event Type │    Score    │
│   (1 byte)  │   (1 byte)  │  (1 byte)   │
└─────────────┴─────────────┴─────────────┘
```

| Field | Size | Description |
|-------|------|-------------|
| Player ID | 1 byte | Affected player (0-3) |
| Event Type | 1 byte | Type of player event |
| Score | 1 byte | Optional score value (0 if not applicable) |

**Player Event Types**:
- `0x00`: Player joined
- `0x01`: Player left/disconnected
- `0x02`: Player died
- `0x03`: Player respawned
- `0x04`: Score updated

---

### 0x13 - PING_RESPONSE

Response to a PING request for latency calculation.

**Direction**: Server → Client

**Payload**: Empty (0 bytes)

**Usage**: The client calculates round-trip time (RTT) by comparing the timestamp in the response with the original PING timestamp:
```
RTT = current_time - original_timestamp
Latency = RTT / 2
```

---

## Connection Flow

### Initial Connection Sequence

```
Client                                Server
  |                                     |
  |  (1) JOIN (username)                |
  |------------------------------------>|
  |                                     |
  |                          (2) Assign Player ID
  |                                     |
  |  (3) PLAYER_ID_ASSIGNMENT           |
  |<------------------------------------|
  |                                     |
  |  (4) PLAYER_EVENT (other players)   |
  |<------------------------------------|
  |                                     |
  |  (5) Broadcast JOIN to others       |
  |                                 [Broadcast]
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
  |  ENTITY_EVENT (spawn, destroy)      |
  |<------------------------------------|
  |                                     |
  |  PLAYER_EVENT (score, death)        |
  |<------------------------------------|
  |                                     |
  |  PING (periodic, every 1-5s)        |
  |------------------------------------>|
  |                                     |
  |  PING_RESPONSE                      |
  |<------------------------------------|
  |                                     |
```

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

Recommended additions:
- Connection timeout for inactive clients
- Rate limiting on JOIN requests per IP
- Packet flood detection

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
```

### Thread Safety

- Client endpoint map (`_clients`) is protected by mutex (`_clientsMutex`)
- Player slots array (`_playerSlots`) access is synchronized
- ASIO's `async_receive_from` ensures safe concurrent packet handling

### UDP Reliability Considerations

Since UDP is unreliable:
- Critical messages (JOIN, PLAYER_ID_ASSIGNMENT) may need retransmission
- Client should retry JOIN if no response within timeout
- Future enhancement: Implement ACK mechanism for reliable messages

---

## Future Extensions

Potential protocol enhancements for Part 2:

1. **Packet Compression**: Add compression flag in header, compress payload with LZ4/zlib
2. **Sequence Acknowledgment**: ACK packets for reliable delivery over UDP
3. **Delta Compression**: Send only changed entity data in SNAPSHOT
4. **Message Fragmentation**: Support for packets > 1024 bytes
5. **Encryption**: Add optional payload encryption for sensitive data
6. **Voice/Text Chat**: New packet types for player communication
7. **Lobby System**: Packets for game room creation, discovery, matchmaking

---

## Appendix: Packet Type Reference Table

| Hex | Dec | Name | Direction | Description |
|-----|-----|------|-----------|-------------|
| 0x01 | 1 | INPUT | C→S | Player input command |
| 0x02 | 2 | JOIN | C→S | Connection request |
| 0x03 | 3 | PING | C→S | Latency measurement |
| 0x08 | 8 | PLAYER_ID_ASSIGNMENT | S→C | Player ID assignment |
| 0x10 | 16 | SNAPSHOT | S→C | Game state update |
| 0x11 | 17 | ENTITY_EVENT | S↔C | Entity event notification |
| 0x12 | 18 | PLAYER_EVENT | S→C | Player event notification |
| 0x13 | 19 | PING_RESPONSE | S→C | Ping acknowledgment |

**Legend**: C = Client, S = Server, → = Unidirectional, ↔ = Bidirectional

---

## Contact & Contribution

For questions, bug reports, or protocol enhancement proposals, please refer to the project's developer documentation and contribution guidelines.

**Protocol Version**: 1.0  
**Last Updated**: January 2025  
**Compatibility**: R-Type Server v1.x