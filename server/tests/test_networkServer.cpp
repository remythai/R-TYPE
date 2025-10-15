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

TEST(NetworkServerTest, CreateAndDestroyPlayerEntity)
{
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
