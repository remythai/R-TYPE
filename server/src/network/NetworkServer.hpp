/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** NetworkServer.hpp
*/

#pragma once
#include <asio.hpp>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
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
    SNAPSHOT = 0x10
};

struct PlayerSlot
{
    bool isUsed;
    uint8_t playerId;
    asio::ip::udp::endpoint endpoint;
    std::string username;
    std::chrono::steady_clock::time_point lastActive;
    EntityManager::Entity entity;
};

class NetworkServer
{
   public:
    NetworkServer(
        unsigned short port, std::string const& hostname,
        std::string const& game);
    ~NetworkServer();

    void run();
    void stop();
    void broadcast(const std::vector<uint8_t>& message);
    int countActivePlayers() const;
    EntityManager::Entity createPlayerEntity(uint8_t playerId);
    void destroyPlayerEntity(uint8_t playerId);
    void setPlayerSlot(size_t index, const PlayerSlot& slot)
    {
        if (index >= _playerSlots.size())
            return;
        std::lock_guard<std::mutex> lock(_playerSlotsMutex);
        _playerSlots[index] = slot;
    }

    std::vector<uint8_t> serializeSnapshot();
    EntityManager::Entity createEnemyEntity();
    void applyInputToEntity(uint8_t playerId, uint8_t keyCode, uint8_t action);
    static std::string packetTypeToString(PacketType type);

   private:
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

    void handleSnapshotPacket(
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

    void sendSnapshot(const asio::ip::udp::endpoint& clientEndpoint);
    void broadcastSnapshot();

    bool _running;
    asio::io_context _ioContext;
    asio::ip::udp::socket _socket;
    std::map<int, asio::ip::udp::endpoint> _clients;
    std::mutex _clientsMutex;
    int _nextClientId = 1;
    std::string _hostname;
    std::string _game;

    std::array<PlayerSlot, 4> _playerSlots;
    std::mutex _playerSlotsMutex;

    std::unique_ptr<Registry> _registry;
    std::chrono::steady_clock::time_point _lastUpdate;

    std::chrono::steady_clock::time_point _lastSnapshot;
    static constexpr float SNAPSHOT_RATE = 1.0f / 20.0f;

    std::mutex _registryMutex;
};
}  // namespace rtype
