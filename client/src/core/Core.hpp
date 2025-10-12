/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.hpp
*/

#pragma once

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <unordered_map>
#include "../network/NetworkClient.hpp"
#include "../macros.hpp"
#include "../graphics/EntityManager.hpp"
#include "../graphics/ParallaxSystem.hpp"
#include "../graphics/AnimatedSprite.hpp"
#include "../graphics/Window.hpp"
#include <SFML/Audio/Music.hpp>

namespace CLIENT {

enum class KeyCode : uint8_t {
    DOWN = 0,
    UP = 1,
    LEFT = 2,
    RIGHT = 3,
    SHOOT = 4
};

enum class InputAction : uint8_t {
    PRESSED = 1,
    RELEASED = 0
};

struct Player {
    AnimatedSprite sprite;
    int playerId;
    bool active;
    sf::Vector2f position;
    int currentRotation;
};

struct Enemy {
    uint32_t entityId;
    uint8_t enemyType;
    bool active;
    sf::Vector2f position;
    sf::Vector2f velocity;
    AnimatedSprite sprite;
};

struct LocalProjectile {
    uint32_t id;
    sf::Vector2f position;
    sf::Vector2f velocity;
    AnimatedSprite sprite;
};

class Core {
public:
    class CoreError : public std::exception {
    private:
        std::string _message;
    public:
        explicit CoreError(const std::string& msg) : _message(msg) {}
        const char* what() const noexcept override { return _message.c_str(); }
    };

    Core(char **argv);
    ~Core();
    
    void run();

private:
    void parseCommandLineArgs(char **argv);
    void initializeNetwork();
    void setupNetworkCallbacks();
    void loadResources();
    
    void networkLoop();
    void graphicsLoop();
    
    void processOutgoingMessages();
    void processIncomingMessages(std::array<Player, 4>& players, Window& window);
    void sendInput(KeyCode keyCode, InputAction action);
    
    void handleKeyStateChange(const std::string& action, bool isPressed, 
                             std::map<std::string, bool>& keyStates);
    void processInputs(Window& window, std::map<std::string, bool>& keyStates);
    
    void initializePlayers(std::array<Player, 4>& players);
    void updateEnemies(std::unordered_map<uint32_t, Enemy>& enemies, float deltaTime);
    void cleanupEnemies(std::unordered_map<uint32_t, Enemy>& enemies);
    void updateProjectiles(std::vector<LocalProjectile>& projectiles, float deltaTime);
    void cleanupProjectiles(std::vector<LocalProjectile>& projectiles);
    
    void spawnTestEnemies(std::unordered_map<uint32_t, Enemy>& enemies, 
                         uint32_t& nextEnemyId);
    void updateEnemySpawning(std::unordered_map<uint32_t, Enemy>& enemies,
                            uint32_t& nextEnemyId, float& spawnTimer, float deltaTime);
    
    void renderScene(Window& window, const std::array<Player, 4>& players,
                    const std::unordered_map<uint32_t, Enemy>& enemies,
                    const std::vector<LocalProjectile>& projectiles);
    
    void parseServerEntities(const std::string& message);
    void parseSnapshot(const std::vector<uint8_t>& payload);

    std::string _hostname;
    unsigned short _port;
    std::string _username;
    uint8_t _myPlayerId;
    
    std::unique_ptr<NetworkClient> _networkClient;
    
    std::thread _networkThread;
    bool _running;
    
    std::queue<std::string> _incomingMessages;
    std::queue<std::string> _outgoingMessages;
    std::mutex _incomingMutex;
    std::mutex _outgoingMutex;

    std::unique_ptr<sf::Music> _backgroundMusic;
    std::unique_ptr<EntityManager> _entityManager;
    std::unique_ptr<ParallaxSystem> _parallaxSystem;
};

Enemy createTestEnemy(uint32_t id, uint8_t type, float x, float y, 
                     ResourceManager& rm);

} // namespace CLIENT

int execute_rtypeClient(char **argv);