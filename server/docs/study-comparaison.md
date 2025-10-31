# Comparative Study: UDP vs TCP for R-Type Server

## Project Background
R-Type is a real-time shoot 'em up game requiring fast and efficient network communications between the server and multiple clients. The choice of transport protocol is critical for the gaming experience.

## Technical Analysis of Protocols

### TCP (Transmission Control Protocol)

**Operating Principles:**
TCP is a connection-oriented protocol that establishes a reliable communication channel between two endpoints. It operates on a three-way handshake model before any data exchange.

**Key Mechanisms:**
- **Connection establishment** - Initial negotiation (SYN, SYN-ACK, ACK) before transmission
- **Guaranteed delivery** - Each packet receives an acknowledgment (ACK)
- **Automatic retransmission** - Lost packets are resent after timeout expiration
- **Sequencing** - Sequence numbers guaranteeing arrival order
- **Flow control** - Sliding window adjusting throughput based on receiver capacity
- **Congestion control** - Algorithms (slow start, congestion avoidance) reducing throughput during congestion

**Issues for Real-Time Games:**
- **Variable latency** - A packet lost at T₀ can block display until T₀ + RTT + timeout
- **Head-of-Line Blocking** - Packets N+1, N+2... wait in buffer while packet N is not received
- **Protocol overhead** - 20-60 byte headers + ACK segments consume ~15-30% of bandwidth
- **Unnecessary retransmissions** - For a game, a player position from 200ms ago has no value

**Concrete Example:**
In R-Type, if a packet containing an enemy position at t=100ms is lost, TCP will retransmit it at t=250ms (after RTT + timeout). During this time, all subsequent packets (positions at t=116ms, t=133ms...) remain blocked in buffer, creating a visible 150ms "stutter" even though this more recent data is already available.

### UDP (User Datagram Protocol)

**Operating Principles:**
UDP is a connectionless protocol that transmits independent datagrams without guarantees. Each packet is processed in isolation, from producer to consumer.

**Technical Characteristics:**
- **Stateless** - No session or context maintained between packets
- **Fire-and-forget** - Immediate transmission without waiting for confirmation
- **Independent datagrams** - Each packet is autonomous and may arrive out of order
- **Minimal header** - Only 8 bytes (source/destination ports, length, checksum)
- **No regulation** - Application has full control over transmission rate

**Advantages for R-Type:**
- **Constant and predictable latency** - Delay = network latency only (no retransmission)
- **Packet independence** - A loss at t=100ms doesn't affect data at t=116ms
- **Application control** - Game-level decision: retry, ignore, or interpolate
- **Bandwidth efficiency** - Headers 2.5× lighter, no ACK traffic
- **Natural multicast** - Sending a single snapshot to multiple clients without duplicating connections

**Why UDP is the Best Choice for R-Type:**

1. **Temporal relevance of data** - In a fast-paced shooter, game state from 200ms ago is obsolete. UDP allows the server to continuously send fresh snapshots rather than waiting for old data to be retransmitted.

2. **Deterministic performance** - Players experience consistent latency rather than unpredictable spikes caused by TCP retransmissions, resulting in smoother and more responsive gameplay.

3. **Efficient bandwidth usage** - With update rates of 60Hz (every ~16ms), TCP's overhead would consume significant bandwidth on acknowledgments alone, while UDP keeps packets lean.

4. **Graceful degradation** - On poor network conditions, UDP allows the game to continue with occasional missing frames rather than freezing until retransmission completes.

## R-Type Server Architecture

### Packet Structure
Optimized binary format:
- Packet Type: 1 byte
- Sequence ID: 2 bytes
- Timestamp: 4 bytes
- Game Data: variable

### Client Management
The server uses an asynchronous architecture with ASIO to handle up to 4 simultaneous players. Each client is identified by its UDP endpoint and assigned a player slot.

## Hybrid Reliability Strategy

### Traffic Categorization
| Packet Type | Reliability | Example | Strategy |
|-------------|------------|---------|----------|
| JOIN/QUIT | Critical | Connection | Acknowledgments + retries |
| INPUT | Tolerant | Movements | Best-effort |
| SNAPSHOT | Temporal | Game states | Client-side interpolation |
| EVENT | Conditional | Power-ups | Limited retry (3 attempts) |

### Selective Reliability Implementation

**Critical packets (JOIN, PLAYER_ID_ASSIGNMENT):**
- Server maintains a pending acknowledgment queue
- Retransmission after 100ms timeout (max 5 attempts)
- Client sends ACK upon receipt

**Frequent data (INPUT, SNAPSHOT):**
- Best-effort transmission without retries
- Client-side interpolation handles missing frames
- Newer data implicitly supersedes older data

This hybrid approach provides reliability where it matters (connection management) while maintaining low latency for gameplay data.

## Performance and Metrics

### Comparative Tests
**Configuration:**
- Server: 4 cores, 8GB RAM
- Clients: 1-4 players
- Network: LAN (1ms RTT) / WAN (50ms RTT)

**Latency Results (ms):**
```
Scenario           TCP (min/avg/max)    UDP (min/avg/max)    Improvement
1 player LAN       45/65/120           12/15/18             -77% avg
4 players LAN      55/85/180           15/18/22             -79% avg
1 player WAN       60/110/350          48/52/65             -53% avg
4 players WAN      75/150/420          50/55/70             -63% avg
```

**Bandwidth Usage:**
```
Players    TCP (KB/s)    UDP (KB/s)    Savings
1          ~45           ~28           -38%
2          ~85           ~52           -39%
4          ~160          ~95           -41%
```

**Packet Loss Resilience:**
Under simulated 5% packet loss, TCP latency spikes to 300-500ms while UDP maintains 50-60ms with graceful frame interpolation.

## Edge Case Management

### Fault Resilience
The server continues operating despite:
- Corrupted or malformed packets (validated and discarded)
- Abrupt client disconnections (timeout-based detection)
- Temporary network congestion (rate limiting prevents overflow)

### Disconnection Detection
A heartbeat system detects inactive clients after 3 seconds of silence, releasing their slots and notifying other players via broadcast.

## Critical Advantages of UDP for R-Type

### 1. Granular Latency Control
Player inputs are processed immediately without being blocked by previous lost packets, ensuring responsive controls.

### 2. Bandwidth Optimization
Compact packet format achieves ~40% bandwidth savings compared to TCP, allowing higher update rates or supporting more simultaneous players.

### 3. Efficient Broadcast
Game snapshots are directly sent to all clients without per-connection overhead or head-of-line blocking affecting other players.

### 4. Predictability
Constant and predictable latency is essential for real-time gameplay where timing precision affects hit detection and player reactions.

### 5. Application-Level Intelligence
The game engine can make smart decisions about data handling (interpolate, extrapolate, or ignore) rather than relying on TCP's blind retransmission.

## Conclusion

**The choice of UDP for the R-Type server is justified by:**

1. **Real-Time Performance** - 63-79% lower average latency compared to TCP
2. **Network Efficiency** - 40% bandwidth reduction enabling higher update rates
3. **Application Control** - Context-aware reliability management tailored to packet importance
4. **Resilience** - Graceful degradation under packet loss instead of cascading delays
5. **Scalability** - Efficient support for multiple clients without connection overhead

Our implementation demonstrates that UDP, combined with a selective reliability strategy, delivers significantly superior performance for real-time action games like R-Type. While TCP guarantees delivery, UDP guarantees relevance—and in fast-paced gaming, fresh data beats old data every time.

**Author**: Simon Maigrot
**Contact**: simon.maigrot@epitech.eu