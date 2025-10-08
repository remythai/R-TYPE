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
#include <unordered_map>
#include <vector>
#include <mutex>
#include <string>

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

            // Handle client packets
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

            bool _running;
            asio::io_context _ioContext;
            asio::ip::udp::socket _socket;
            std::map<int, asio::ip::udp::endpoint> _clients;
            std::mutex _clientsMutex;
            int _nextClientId = 1;
            std::string _hostname;

            std::array<PlayerSlot, 4> _playerSlots;
            std::mutex _playerSlotsMutex;
    };
}
