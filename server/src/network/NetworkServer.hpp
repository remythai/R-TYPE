/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkServer.hpp
*/

#pragma once
#include <asio.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "../../../gameEngine/components/AIControlled/src/AIControlled.hpp"
#include "../../../gameEngine/components/acceleration/src/Acceleration.hpp"
#include "../../../gameEngine/components/domain/src/Domain.hpp"
#include "../../../gameEngine/components/inputControlled/src/InputControlled.hpp"
#include "../../../gameEngine/components/position/src/Position.hpp"
#include "../../../gameEngine/components/renderable/src/Renderable.hpp"
#include "../../../gameEngine/components/velocity/src/Velocity.hpp"
#include "../../../gameEngine/ecs/Registry.hpp"
#include "../../../gameEngine/systems/FPApplyGravity/src/FPApplyGravity.hpp"
#include "../../../gameEngine/systems/FPInputHandler/src/FPInputHandler.hpp"
#include "../../../gameEngine/systems/FPMotion/src/FPMotion.hpp"
#include "../../../gameEngine/systems/animation/src/Animation.hpp"
#include "../../../gameEngine/systems/collision/src/Collision.hpp"
#include "../../../gameEngine/systems/death/src/Death.hpp"
#include "../../../gameEngine/systems/domainHandler/src/DomainHandler.hpp"
#include "../../../gameEngine/systems/inputHandler/src/InputHandler.hpp"
#include "../../../gameEngine/systems/motion/src/Motion.hpp"

namespace rtype {
enum class PacketType : uint8_t
{
    INPUT = 0x01,
    JOIN = 0x02,
    PLAYER_ID_ASSIGNMENT = 0x08,
    SNAPSHOT = 0x10,
    TIMEOUT = 0x20,
    KILLED = 0x40
};

/**
 * @brief Structure to hold player slot information
 */
struct PlayerSlot
{
    bool isUsed;
    uint8_t playerId;
    asio::ip::udp::endpoint endpoint;
    std::string username;
    std::chrono::steady_clock::time_point lastActive;
    EntityManager::Entity entity;
};

/**
 * @brief Structure to hold enemy spawn data
 */
struct EnemySpawnData
{
    int type;
    float x;
    float y;
    float spawnTime;
    std::string spritePath;
    std::array<float, 4> textureRect;
};

class NetworkServer
{
   public:
    class NetworkServerError : public std::exception
    {
       private:
        std::string _msg;

       public:
        explicit NetworkServerError(const std::string& msg) : _msg(msg) {}
        const char* what() const noexcept override
        {
            return _msg.c_str();
        }
    };
    NetworkServer(unsigned short port, std::string const& game);
    ~NetworkServer();

    void run();

    void broadcast(const std::vector<uint8_t>& message);

    int countActivePlayers() const;

    EntityManager::Entity createPlayerEntity(uint8_t playerId);
    void applyInputToEntity(uint8_t playerId, uint8_t keyCode, uint8_t action);
    void destroyPlayerEntity(uint8_t playerId);
    EntityManager::Entity createEnemyEntity();

    /**
     * @brief Sets a player slot at the specified index
     *
     * @param index The index of the player slot (0-3)
     * @param slot The PlayerSlot data to set
     */
    void setPlayerSlot(size_t index, const PlayerSlot& slot)
    {
        if (index >= _playerSlots.size())
            return;
        std::lock_guard<std::mutex> lock(_playerSlotsMutex);
        _playerSlots[index] = slot;
    }

    std::vector<uint8_t> serializeSnapshot();
    static std::string packetTypeToString(PacketType type);

   private:
    /**
     * @brief Converts a value of type T to a vector of bytes
     *
     * @tparam T The type of the value to convert
     * @param value The value to convert
     * @return std::vector<uint8_t> The byte representation of the value
     */
    template <typename T>
    std::vector<uint8_t> toBytes(T value)
    {
        std::vector<uint8_t> bytes(sizeof(T));
        for (size_t i = 0; i < sizeof(T); ++i)
            bytes[sizeof(T) - 1 - i] = (value >> (i * 8)) & 0xFF;
        return bytes;
    }

    /**
     * @brief Converts a byte array to a value of type T
     *
     * @tparam T The type of the value to convert to
     * @param data Pointer to the byte array
     * @return T The converted value
     */
    template <typename T>
    T fromBytes(const uint8_t* data)
    {
        T value = 0;
        for (size_t i = 0; i < sizeof(T); ++i)
            value |= data[i] << (8 * (sizeof(T) - 1 - i));
        return value;
    }

    void doReceive();

    void initECS();
    void updateECS(float dt);

    void handleClientPacket(
        const asio::ip::udp::endpoint& clientEndpoint, PacketType type,
        uint16_t packetId, uint32_t timestamp,
        const std::vector<uint8_t>& payload);

    void handleInputPacket(
        const asio::ip::udp::endpoint& clientEndpoint,
        const std::vector<uint8_t>& payload);

    void handleJoinPacket(
        const asio::ip::udp::endpoint& clientEndpoint,
        const std::vector<uint8_t>& payload);

    void sendPlayerIdAssignment(
        const asio::ip::udp::endpoint& clientEndpoint, uint8_t playerId);

    uint8_t findPlayerIdByEndpoint(const asio::ip::udp::endpoint& endpoint);

    void checkInactivePlayers();
    void cleanInactivePlayers();

    void broadcastSnapshot();

    bool _running;
    std::string _hostname;
    asio::io_context _ioContext;
    asio::ip::udp::socket _socket;
    std::map<int, asio::ip::udp::endpoint> _clients;
    std::mutex _clientsMutex;
    int _nextClientId = 1;
    std::string _game;

    std::array<PlayerSlot, 4> _playerSlots;
    std::mutex _playerSlotsMutex;

    std::unique_ptr<Registry> _registry;
    std::chrono::steady_clock::time_point _lastUpdate;

    std::chrono::steady_clock::time_point _lastSnapshot;
    static constexpr float SNAPSHOT_RATE = 1.0f / 20.0f;

    std::mutex _registryMutex;

    std::vector<EnemySpawnData> _enemySpawnList;
    float _gameTime = 0.0f;
    size_t _nextEnemyToSpawn = 0;

    void loadEnemiesFromJson(const std::string& filepath);
    void checkAndSpawnEnemies();
    EntityManager::Entity createEnemyFromData(const EnemySpawnData& data);
    void handlePlayerDeath(EntityManager::Entity entity);
};
}  // namespace rtype
