# Data Persistence Comparison: JSON vs Alternatives

## Executive Summary

This document analyzes data persistence solutions available for the R-Type game engine, comparing JSON (current choice) against relational databases, binary formats, and other structured storage formats. The analysis justifies JSON as the optimal choice for game configuration while identifying scenarios where alternatives would be more appropriate.

---

## Table of Contents

1. [Persistence Requirements for R-Type](#persistence-requirements-for-r-type)
2. [Available Solutions Overview](#available-solutions-overview)
3. [Detailed Technology Comparison](#detailed-technology-comparison)
4. [Current Implementation: JSON](#current-implementation-json)
5. [Alternative Scenarios](#alternative-scenarios)
6. [Recommendations](#recommendations)

---

## Persistence Requirements for R-Type

### Data Categories

R-Type requires persistence for three distinct data categories:

#### 1. Game Configuration (Static)
- **Map/Level definitions**: Enemy spawn patterns, obstacles
- **Game balance**: Fire rates, movement speeds, health values
- **Asset references**: Sprite paths, audio files, fonts
- **Characteristics**:
  - Human-editable
  - Loaded once at startup
  - ~1-10 MB typical size
  - Rarely changes during runtime
  - Version control friendly

#### 2. Game State (Dynamic)
- **Player progress**: Scores, completed levels
- **Game sessions**: Active game instance state
- **Characteristics**:
  - Changes frequently during gameplay
  - Needs fast read/write (60 FPS)
  - Medium data volume (~100 KB per game)
  - Must be reliable (don't lose player progress)

#### 3. User Preferences (Semi-Static)
- **Keybinds**: Custom control mappings
- **Graphics settings**: Resolution, quality levels
- **Audio settings**: Volume levels
- **Characteristics**:
  - User-configurable
  - Loaded at startup, saved on exit
  - Small data volume (~10 KB)
  - Rarely accessed

### Performance Constraints

| Category | Read Latency | Write Latency | Throughput | Frequency |
|----------|---|---|---|---|
| Configuration | ~100ms (startup) | N/A | N/A | Once at startup |
| Game State | <16ms (60 FPS) | <16ms (60 FPS) | 100+ updates/sec | Every frame |
| Preferences | ~100ms (startup) | ~100ms (on exit) | N/A | Rare |

---

## Available Solutions Overview

```
┌─────────────────────────────────────────────────────────┐
│              PERSISTENCE SOLUTIONS                       │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  STRUCTURED FORMATS:                                    │
│  ├─ JSON (✓ Current choice)                             │
│  ├─ XML                                                  │
│  ├─ YAML                                                │
│  ├─ Protocol Buffers                                    │
│  └─ MessagePack                                         │
│                                                          │
│  DATABASES:                                             │
│  ├─ SQLite (Relational)                                 │
│  ├─ MongoDB (Document)                                  │
│  ├─ Redis (In-Memory)                                   │
│  └─ LevelDB (Key-Value)                                 │
│                                                          │
│  BINARY FORMATS:                                        │
│  ├─ Custom Binary                                       │
│  ├─ Flatbuffers                                         │
│  └─ Cap'n Proto                                         │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

---

## Detailed Technology Comparison

### 1. JSON (SELECTED)

#### File Structure Example
```json
{
  "level": 1,
  "enemies": [
    {
      "type": "BasicEnemy",
      "spawnTime": 2.5,
      "position": {"x": 1920, "y": 540},
      "health": 100
    },
    {
      "type": "SinusoidalEnemy",
      "spawnTime": 5.0,
      "position": {"x": 1920, "y": 300},
      "health": 150
    }
  ],
  "objectives": {
    "surviveSeconds": 120,
    "killCount": 50
  }
}
```

#### Advantages ✅

**1. Human-Readable**
- Game designers can edit levels directly in text editor
- Diffs are meaningful in version control
- Easy debugging (no tool required)
- Non-technical team members can validate data

**2. Widely Supported**
- Standard library available in C++ (nlohmann/json)
- Human-readable format
- Industry standard for game config (Unity, Unreal, Godot)

**3. Language Independent**
- Same format usable across server, client, tools
- Easy to share data between different systems
- Cross-platform compatibility guaranteed

**4. Schema Flexibility**
- Easy to add/remove fields without breaking existing files
- Supports nested structures naturally
- No schema compilation required
- Forward/backward compatible

**5. Tooling Ecosystem**
- JSON validators, formatters, visualizers available
- IDE support (syntax highlighting, autocompletion)
- Easy to generate from other sources

#### Disadvantages ❌

**1. Parse Performance**
- Text parsing slower than binary formats
- Typical: 10-50 MB/s parsing speed
- **Impact for R-Type**: ~10ms for 1 MB file (acceptable at startup)

**2. File Size Overhead**
- ~2-3× larger than binary formats
- 1 MB JSON config → 300-500 KB binary equivalent
- **Impact for R-Type**: Negligible (game configs rarely >10MB)

**3. Type Ambiguity**
- JSON numbers don't distinguish int/float/long
- Requires application-level type inference
- Can cause precision loss (64-bit integers → doubles)

**4. No Schema Enforcement**
- Typos or missing fields not caught at load time
- Requires runtime validation code
- **Example**:
  ```json
  {
    "firerRate": 0.33  // Typo! Should be "fireRate"
    // No error until game tries to read it
  }
  ```

#### Implementation in R-Type

**Files**:
- `map_level1.json`, `map_level2.json`, `map_level3.json` (enemy spawn patterns)
- Server loads on startup via nlohmann/json
- Example (server/src/network/loadEnemies.cpp):

```cpp
std::vector<Enemy> loadEnemies(const std::string& filePath) {
    std::ifstream file(filePath);
    auto data = nlohmann::json::parse(file);

    std::vector<Enemy> enemies;
    for (const auto& enemy : data["enemies"]) {
        enemies.push_back({
            enemy["type"].get<std::string>(),
            enemy["spawnTime"].get<float>(),
            enemy["health"].get<int>()
        });
    }
    return enemies;
}
```

#### Metrics

| Metric | Value | Notes |
|--------|-------|-------|
| Parse Speed | 10-50 MB/s | Depends on complexity |
| File Size | 1.0 (baseline) | Relative to optimal |
| Load Time (10MB) | ~10-100ms | Acceptable for game startup |
| Human Readability | Excellent | Easily editable |
| Type Safety | Low | Application-level checking needed |

---

### 2. XML

#### File Structure Example
```xml
<?xml version="1.0"?>
<level id="1">
  <enemies>
    <enemy type="BasicEnemy" spawnTime="2.5">
      <position x="1920" y="540"/>
      <health>100</health>
    </enemy>
  </enemies>
  <objectives surviveSeconds="120" killCount="50"/>
</level>
```

#### Advantages ✅
- Standard for many enterprise systems
- Excellent tooling (XML editors, validators, XSD schemas)
- DTD/XSD schema support enforces structure
- Wide library support

#### Disadvantages ❌
- **Verbose**: 4-5× larger than JSON for equivalent data
- **Complexity**: Attributes vs elements creates ambiguity
- **Slower**: ~2× slower to parse than JSON
- **Overkill**: Over-engineered for game config
- **Not game-industry standard**: Rarely used in modern games

#### Verdict: ❌ Worse than JSON
- All of JSON's advantages with extra verbosity
- Would only make files harder to edit

---

### 3. YAML

#### File Structure Example
```yaml
level: 1
enemies:
  - type: BasicEnemy
    spawnTime: 2.5
    position:
      x: 1920
      y: 540
    health: 100
  - type: SinusoidalEnemy
    spawnTime: 5.0
    position: {x: 1920, y: 300}
    health: 150
objectives:
  surviveSeconds: 120
  killCount: 50
```

#### Advantages ✅
- **Most human-readable**: Minimal syntax
- **Supports comments**: `# This is a comment`
- **Flexible**: Multiple ways to express same data (good or bad)
- **Popular in DevOps**: Used by Kubernetes, Ansible, Docker Compose

#### Disadvantages ❌
- **Slower parsing**: 5-10× slower than JSON
- **Ambiguity**: Whitespace-sensitive, easy to break
- **Library support**: Fewer C++ libraries (YAML isn't built-in)
- **Performance overhead**: ~50-100ms for medium file

#### R-Type Assessment
```
Parsing time for 1MB YAML: ~50-100ms
Parsing time for 1MB JSON:  ~10-50ms

For game startup: Minimal impact
For runtime: Would be unacceptable if loaded every frame
```

#### Verdict: ⚠️ Optional Alternative for Configuration
- Could replace JSON for config files (not runtime data)
- Better readability for non-technical designers
- Small performance cost at startup is acceptable
- Not suitable for high-frequency data

---

### 4. Protocol Buffers (protobuf)

#### Structure Definition
```protobuf
syntax = "proto3";

message Enemy {
  string type = 1;
  float spawnTime = 2;
  Position position = 3;
  int32 health = 4;
}

message Position {
  int32 x = 1;
  int32 y = 2;
}

message Level {
  int32 id = 1;
  repeated Enemy enemies = 2;
  Objectives objectives = 3;
}
```

#### Advantages ✅
- **Binary format**: 3-5× smaller than JSON
- **Fast parsing**: 100-200 MB/s
- **Type-safe**: Schema enforced at compile time
- **Versioning**: Forward/backward compatible
- **Multiple languages**: Generate code for C++, Python, Java

#### Disadvantages ❌
- **Not human-readable**: Binary format requires tools to inspect
- **Compilation step**: Must compile .proto files first
- **Build complexity**: Adds build system dependency
- **Overkill**: Over-engineered for game config

#### Use Case
```
Ideal for:
- High-frequency network protocol (game server state)
- Multi-language microservices
- When performance is critical

Not ideal for:
- Game config (needs human editability)
- One-time startup files
- Small project teams
```

#### Verdict: ⚠️ Consider for Game State Persistence (Not Config)

If R-Type implemented player progress saves:
```cpp
message GameSave {
  int32 playerScore = 1;
  int32 level = 2;
  float playTime = 3;
  repeated Achievement achievements = 4;
}

// Binary size: ~100 bytes (vs 500+ for JSON)
// Parse time: <1ms (vs 5-10ms for JSON)
```

---

### 5. MessagePack

#### Binary Format Example
```
Original JSON:
{"name": "Player", "score": 1000, "level": 5}

MessagePack (hex): 83 a4 6e 61 6d 65 a6 50 6c 61 79 65 72 a5 73 63 6f 72 65 cd 03 e8 a5 6c 65 76 65 6c 05

Size: JSON=32 bytes → MessagePack=28 bytes
```

#### Advantages ✅
- **Compact binary**: 20-30% smaller than JSON
- **Fast parsing**: 50-100 MB/s
- **Maintains schema flexibility**: Like JSON, no schema required
- **Language support**: C++, Python, JavaScript, etc.

#### Disadvantages ❌
- **Less common**: Fewer developers familiar with format
- **Less tooling**: Hard to inspect/edit binary files
- **No schema**: Same validation challenges as JSON
- **Minimal benefit for game config**: Size savings negligible for startup files

#### Verdict: ⚠️ Only if file size is critical

For R-Type:
- Config files typically <10 MB
- Load at startup (no real-time constraint)
- Benefit doesn't justify losing human-editability

---

### 6. SQLite

#### Schema Example
```sql
CREATE TABLE levels (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    duration INTEGER
);

CREATE TABLE enemies (
    id INTEGER PRIMARY KEY,
    level_id INTEGER,
    type TEXT NOT NULL,
    spawn_time REAL,
    health INTEGER,
    FOREIGN KEY (level_id) REFERENCES levels(id)
);

CREATE TABLE player_progress (
    player_id INTEGER PRIMARY KEY,
    current_level INTEGER,
    total_score INTEGER,
    last_played TIMESTAMP,
    FOREIGN KEY (current_level) REFERENCES levels(id)
);
```

#### Advantages ✅
- **Powerful queries**: SQL allows complex data retrieval
- **ACID guarantees**: Reliable data consistency
- **Schema enforcement**: Data types verified
- **Scalable**: Efficient with large datasets
- **Built-in**: No extra dependencies (SQLite bundled with most systems)

#### Disadvantages ❌
- **Overkill for static config**: Game maps rarely change post-deployment
- **Not human-editable**: Requires database tool to inspect/modify
- **Version control unfriendly**: Binary format causes huge diffs
- **Query overhead**: Slower than direct file load
- **Cold start**: Connection overhead at game startup

#### Typical Query Performance
```
Operation               | Time      | Notes
Load entire config      | 5-20ms    | Slower than JSON parse
Query specific enemy    | 1-2ms     | Fast if indexed
Insert new player save  | 2-5ms     | Fast, ACID guaranteed
Update player score     | 1-3ms     | Efficient for frequent updates
```

#### Verdict: ✅ Excellent for Player Progress, ❌ Wrong for Config

**Recommended Architecture**:
```
Game Configuration: JSON ← Human-editable, static
Player Progress:    SQLite ← ACID-safe, queryable
Runtime State:      Memory ← Fast, ephemeral
```

#### Implementation Example
```cpp
// Config: Load from JSON at startup
std::vector<Enemy> configEnemies = loadJSON("levels/level1.json");

// Player progress: Use SQLite for persistence
auto db = sqlite3_open(":memory:");
sqlite3_exec(db, "CREATE TABLE saves(...)", ...);

// Game state: Keep in memory during gameplay
registry.emplace<Health>(playerEntity, 100, 100);
```

---

### 7. Custom Binary Format

#### Example Layout
```
[FileHeader (4 bytes)]
  └─ Magic: 0x5254YPE (identify as R-Type binary)
  └─ Version: 0x01
  └─ Reserved: 0x00 0x00

[Level Data (variable)]
  ├─ Level ID (1 byte)
  ├─ Enemy Count (2 bytes)
  └─ Enemy Array (per enemy):
      ├─ Type ID (1 byte)          ← Enum: 0=Basic, 1=Sinusoidal
      ├─ Spawn Time (4 bytes, float)
      ├─ Position X (4 bytes, float)
      ├─ Position Y (4 bytes, float)
      └─ Health (2 bytes, int16)

Total per enemy: 16 bytes (vs 80+ JSON)
```

#### Advantages ✅
- **Maximum performance**: Parse at memory copy speed (~1000 MB/s)
- **Minimal size**: 2-3× more compact than JSON
- **Zero parsing**: Memory-mapped access possible
- **Fixed layout**: CPU cache optimal

#### Disadvantages ❌
- **Complete loss of human-editability**: Binary format unreadable
- **Complex version management**: Format changes break compatibility
- **Development burden**: Must maintain serialization code
- **Debugging nightmare**: Hard to inspect data
- **No tooling**: Can't use off-the-shelf editors
- **Not version control friendly**: All binary diffs are noise

#### Verdict: ❌ Avoid for R-Type

Only use custom binary if:
1. File size is critical (>100 MB)
2. Parse speed is critical (<1ms required)
3. Format is stable/frozen
4. Tooling can be built

**R-Type Assessment**: None of these apply
- Config files: <10 MB (not critical)
- Startup only: <100ms acceptable (not critical)
- Format evolving: Need flexibility
- Small team: Can't maintain custom tooling

---

### 8. Flatbuffers vs Cap'n Proto

These are advanced binary formats used by high-performance systems:

#### Flatbuffers
- **Use case**: High-performance game networking
- **Advantage**: Lazy deserialization (don't parse until needed)
- **Drawback**: Overkill for game config

#### Cap'n Proto
- **Use case**: RPC frameworks, microservices
- **Advantage**: Zero-copy deserialization
- **Drawback**: Complex build system

#### Verdict: ❌ Over-engineered for R-Type

---

## Current Implementation: JSON

### Files in R-Type

```
/home/louka/coding/R-TYPE/
├── map_level1.json    ← Enemy spawn patterns
├── map_level2.json
├── map_level3.json
└── gameEngine/docs/data-persistence-comparison.md
```

### Load Process

```cpp
// server/src/network/loadEnemies.cpp

std::vector<Enemy> enemies;

// 1. Open file
std::ifstream levelFile(filename);

// 2. Parse JSON
auto jsonData = nlohmann::json::parse(levelFile);

// 3. Iterate entities
for (const auto& enemy : jsonData["enemies"]) {
    Enemy e{
        enemy["type"].get<std::string>(),
        enemy["spawnTime"].get<float>(),
        enemy["position"]["x"].get<float>(),
        enemy["position"]["y"].get<float>(),
        enemy["health"].get<int>()
    };
    enemies.push_back(e);
}

// 4. Server creates entities in ECS registry
for (const auto& e : enemies) {
    auto entity = registry.create();
    registry.emplace<Position>(entity, e.pos.x, e.pos.y);
    registry.emplace<Health>(entity, e.health, e.health);
    registry.emplace<AIControlled>(entity);
    // ... additional components
}
```

### Performance Profile

```
Scenario: Load map_level1.json (assume 50 enemies)

File size:         ~15 KB
Parse time:        ~2 ms
Entity creation:   ~50 * 0.1ms = 5 ms
Total startup:     ~7 ms (acceptable)

Frequency: Once per game start
Impact:    Negligible
```

### Validation Example

```cpp
// Runtime validation (required because JSON is loosely typed)

void validateEnemy(const nlohmann::json& enemy) {
    if (!enemy.contains("type")) {
        throw std::runtime_error("Enemy missing 'type' field");
    }
    if (!enemy["type"].is_string()) {
        throw std::runtime_error("Enemy 'type' must be string");
    }
    if (enemy["type"] != "BasicEnemy" && enemy["type"] != "SinusoidalEnemy") {
        throw std::runtime_error("Unknown enemy type: " + enemy["type"].get<std::string>());
    }

    if (!enemy.contains("spawnTime") || !enemy["spawnTime"].is_number()) {
        throw std::runtime_error("Enemy missing or invalid 'spawnTime'");
    }

    // Additional checks...
}
```

---

## Alternative Scenarios

### Scenario 1: Player Progress Persistence

**Requirement**: Save/load player scores, completed levels

**Current**: Stored in memory only (lost on disconnect)

**Recommended**: SQLite with checkpoint saves

```cpp
struct PlayerProgress {
    int playerId;
    int level;
    int totalScore;
    std::vector<Achievement> achievements;
    std::time_t lastSaved;
};

// Save to SQLite every 5 minutes
void savePlayerProgress(int playerId, const PlayerProgress& p) {
    auto stmt = db.prepare("INSERT OR REPLACE INTO player_progress VALUES (?, ?, ?, ?)");
    stmt.bind(playerId, p.level, p.totalScore, p.lastSaved);
    stmt.execute();
}
```

**Why**: ACID guarantees prevent progress loss

---

### Scenario 2: Live Game State Streaming

**Requirement**: 60 FPS state synchronization (covered by UDP snapshots)

**Current**: Binary protocol over UDP

**Alternative**: MessagePack compression

```
Current binary snapshot: 256 bytes
+ 60 snapshots/second
= 15.36 KB/second per client

MessagePack: 20% smaller
= 12.29 KB/second per client
= 3.07 KB/second saved per client
```

**Verdict**: Marginal benefit; current approach optimal

---

### Scenario 3: Large World Expansion

**Requirement**: Support 1000+ enemies, 100+ levels (not current scope)

**Recommended Approach**:

```
Streaming system:
├─ Metadata (JSON): Level list, enemy types, quest info
├─ Geometry cache (Binary): Pre-compiled spatial data
├─ Asset registry (SQLite): Sprites, sounds, fonts
└─ Runtime state (Memory): Active entities, player progress
```

**Why**: Hybrid approach balances performance and maintainability

---

## Recommendations

### For Current R-Type Implementation

#### ✅ Continue Using JSON for:
1. **Map/Level configurations** (static, human-editable)
2. **Game balance constants** (fire rates, health values)
3. **Asset manifests** (sprite paths, metadata)

**Why**:
- Human-editable (non-technical designers can modify)
- Version control friendly (meaningful diffs)
- Zero-setup (built into C++ with nlohmann/json)
- Sufficient performance (loaded once at startup)

#### ⚠️ Consider Adding SQLite for:
1. **Player progress** (if multiplayer leaderboards added)
2. **Session history** (if match recording desired)
3. **Achievement tracking** (if progression system added)

**Why**:
- ACID guarantees prevent data loss
- Query support for leaderboards/analytics
- Efficient for frequent read/write

#### ❌ Don't Use:
- Binary formats (lose editability)
- XML (too verbose)
- Databases for config (overthinking)

### Future: Extensible Format

As project grows, consider:

```cpp
// Configuration loader supporting multiple formats

class ConfigLoader {
    enum Format { JSON, YAML, BINARY };

public:
    LevelConfig load(const std::string& path, Format fmt) {
        switch (fmt) {
            case Format::JSON:   return loadJSON(path);
            case Format::YAML:   return loadYAML(path);
            case Format::BINARY: return loadBinary(path);
        }
    }
};

// Auto-detect format from file extension
LevelConfig config = loader.load("level1.json");  // Uses JSON
LevelConfig config = loader.load("level2.yaml");  // Uses YAML
```

---

## Conclusion

**Current Choice (JSON): Optimal for R-Type**

| Criterion | JSON | SQLite | Binary | Alternative |
|-----------|------|--------|--------|-------------|
| Startup Config | ✅✅ | ❌ | ✅ | YAML (⚠️) |
| Player Progress | ⚠️ | ✅✅ | ❌ | MessagePack (⚠️) |
| Performance | ✅ | ⚠️ | ✅✅ | - |
| Editability | ✅✅ | ❌ | ❌ | - |
| Version Control | ✅✅ | ❌ | ❌ | - |
| Developer Time | ✅✅ | ⚠️ | ❌ | - |

**Recommendation**: Maintain JSON for current scope; add SQLite when multiplayer features requiring persistence are implemented.
