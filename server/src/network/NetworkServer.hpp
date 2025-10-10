#pragma once
#include <asio.hpp>
#include <atomic>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <string>
#include "../../../gameEngine/ecs/Registry.hpp"
#include "../../../gameEngine/ecs/components/inputControlled/src/InputControlled.hpp"
#include "../../../gameEngine/ecs/components/acceleration/src/Acceleration.hpp"
#include "../../../gameEngine/ecs/components/position/src/Position.hpp"
#include "../../../gameEngine/ecs/components/velocity/src/Velocity.hpp"
#include "../../../gameEngine/ecs/components/renderable/src/Renderable.hpp"
#include "../../../gameEngine/ecs/systems/inputHandler/src/InputHandler.hpp"
#include "../../../gameEngine/ecs/systems/motion/src/Motion.hpp"

namespace rtype {
    enum class PacketType : uint8_t {
        INPUT = 0x01,
        JOIN = 0x02,
        PING = 0x03,
        SNAPSHOT = 0x10,
        ENTITY_EVENT = 0x11,
        PLAYER_EVENT = 0x12,
        PLAYER_ID_ASSIGNMENT = 0x08,
        PLAYER_LIST = 0x09,
        DISCONNECT = 0x0A,
        PING_RESPONSE = 0x0B
    };

    struct PlayerSlot {
        bool isUsed;
        uint8_t playerId;
        asio::ip::udp::endpoint endpoint;
        std::string username;
        std::chrono::steady_clock::time_point lastActive;
        EntityManager::Entity entity;
    };

    class NetworkServer {
        public:
            NetworkServer(unsigned short port, std::string const &hostname);
            ~NetworkServer();

            void run();
            void stop();
            void broadcast(const std::vector<uint8_t>& message);

        private:
            void doReceive();

            void initECS();
            void updateECS(float dt);
            EntityManager::Entity createPlayerEntity(uint8_t playerId);
            void destroyPlayerEntity(uint8_t playerId);
            void applyInputToEntity(uint8_t playerId, uint8_t keyCode, uint8_t action);

            void handleClientPacket(
                const asio::ip::udp::endpoint& clientEndpoint,
                PacketType type, uint16_t packetId, uint32_t timestamp,
                const std::vector<uint8_t>& payload
            );

            void handleInputPacket(
                const asio::ip::udp::endpoint& clientEndpoint,
                const std::vector<uint8_t>& payload
            );

            void handlePingPacket(
                const asio::ip::udp::endpoint& clientEndpoint,
                uint16_t packetId, uint32_t timestamp
            );

            void handleSnapshotPacket(
                const asio::ip::udp::endpoint& clientEndpoint,
                const std::vector<uint8_t>& payload
            );

            void handleJoinPacket(
                const asio::ip::udp::endpoint& clientEndpoint,
                const std::vector<uint8_t>& payload
            );

            void handleEntityEventPacket(
                const asio::ip::udp::endpoint& clientEndpoint,
                const std::vector<uint8_t>& payload
            );

            void handlePlayerEventPacket(
                const asio::ip::udp::endpoint& clientEndpoint,
                const std::vector<uint8_t>& payload
            );

            void sendPlayerIdAssignment(
                const asio::ip::udp::endpoint& clientEndpoint,
                uint8_t playerId
            );

            void sendPlayerJoinEvent(
                const asio::ip::udp::endpoint& clientEndpoint,
                uint8_t playerId
            );

            uint8_t findPlayerIdByEndpoint(const asio::ip::udp::endpoint& endpoint);
            int countActivePlayers() const;

            std::vector<uint8_t> serializePingResponse(uint16_t packetId, uint32_t timestamp);
            std::string packetTypeToString(PacketType type);
            void checkInactivePlayers();

            void cleanInactivePlayers();

            void sendSnapshot(const asio::ip::udp::endpoint& clientEndpoint);
            void broadcastSnapshot();
            std::vector<uint8_t> serializeSnapshot();

            bool _running;
            asio::io_context _ioContext;
            asio::ip::udp::socket _socket;
            std::map<int, asio::ip::udp::endpoint> _clients;
            std::mutex _clientsMutex;
            int _nextClientId = 1;
            std::string _hostname;

            std::array<PlayerSlot, 4> _playerSlots;
            std::mutex _playerSlotsMutex;

            std::unique_ptr<Registry> _registry;
            std::chrono::steady_clock::time_point _lastUpdate;

            std::chrono::steady_clock::time_point _lastSnapshot;
            static constexpr float SNAPSHOT_RATE = 1.0f / 20.0f;
    };
}
