/*
** EPITECH PROJECT, 2025
** r-type-mirror
** File description:
** test_networkServer.cpp
*/

#include <gtest/gtest.h>
#include "../src/network/NetworkServer.hpp"

TEST(NetworkServerTest, CanInstantiate) {
    rtype::NetworkServer server(0, "127.0.0.1");
    EXPECT_EQ(server.countActivePlayers(), 0);
}

TEST(NetworkServerTest, CreateAndDestroyPlayerEntity) {
    rtype::NetworkServer server(0, "127.0.0.1");

    auto entity = server.createPlayerEntity(0);
    EXPECT_NE(entity, EntityManager::INVALID_ENTITY);

    rtype::PlayerSlot slot;
    slot.isUsed = true;
    slot.entity = entity;
    server.setPlayerSlot(0, slot);

    EXPECT_EQ(server.countActivePlayers(), 1);

    server.destroyPlayerEntity(0);

    slot.isUsed = false;
    slot.entity = EntityManager::INVALID_ENTITY;
    server.setPlayerSlot(0, slot);

    EXPECT_EQ(server.countActivePlayers(), 0);
}

TEST(NetworkServerTest, PacketTypeToStringWorks) {
    EXPECT_EQ(rtype::NetworkServer::packetTypeToString(rtype::PacketType::INPUT), "INPUT");
    EXPECT_EQ(rtype::NetworkServer::packetTypeToString(rtype::PacketType::JOIN), "JOIN");
    EXPECT_EQ(rtype::NetworkServer::packetTypeToString(static_cast<rtype::PacketType>(255)), "UNKNOWN");
}

TEST(NetworkServerTest, SerializePingResponseHasCorrectSizeAndHeader) {
    rtype::NetworkServer server(0, "127.0.0.1");

    auto packet = server.serializePingResponse(123, 456);
    EXPECT_EQ(packet.size(), 7);
    EXPECT_EQ(packet[0], static_cast<uint8_t>(rtype::PacketType::PING_RESPONSE));
}

TEST(NetworkServerTest, CreateEnemyEntityCreatesValidEntity) {
    rtype::NetworkServer server(0, "127.0.0.1");

    auto entity = server.createEnemyEntity();
    EXPECT_NE(entity, EntityManager::INVALID_ENTITY);
}

TEST(NetworkServerTest, SerializeSnapshotReturnsValidPacket) {
    rtype::NetworkServer server(0, "127.0.0.1");

    auto entity = server.createPlayerEntity(0);
    rtype::PlayerSlot slot;
    slot.isUsed = true;
    slot.entity = entity;
    server.setPlayerSlot(0, slot);

    auto snapshot = server.serializeSnapshot();

    ASSERT_GE(snapshot.size(), 7);
    EXPECT_EQ(snapshot[0], static_cast<uint8_t>(rtype::PacketType::SNAPSHOT));
}

TEST(NetworkServerTest, ApplyInputToEntityDoesNotCrash) {
    rtype::NetworkServer server(0, "127.0.0.1");

    auto entity = server.createPlayerEntity(0);
    rtype::PlayerSlot slot;
    slot.isUsed = true;
    slot.entity = entity;
    server.setPlayerSlot(0, slot);

    server.applyInputToEntity(0, 32, 1);
    server.applyInputToEntity(0, 32, 0);

    SUCCEED();
}

TEST(NetworkServerTest, CountActivePlayersReflectsUsage) {
    rtype::NetworkServer server(0, "127.0.0.1");

    rtype::PlayerSlot slot;
    slot.isUsed = true;
    slot.entity = server.createPlayerEntity(0);
    server.setPlayerSlot(0, slot);

    EXPECT_EQ(server.countActivePlayers(), 1);

    slot.isUsed = false;
    server.setPlayerSlot(0, slot);

    EXPECT_EQ(server.countActivePlayers(), 0);
}
