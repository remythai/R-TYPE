/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.hpp - Header with generalized entity system
*/

#pragma once

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <SFML/Audio.hpp>
#include "../network/NetworkClient.hpp"
#include "../graphics/EntityManager.hpp"
#include "../graphics/ParallaxSystem.hpp"
#include "../mapEditor/MapEditor.hpp"

namespace CLIENT {

class Window;

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

class Core {
public:
    class CoreError : public std::exception {
    private:
        std::string _msg;
    public:
        explicit CoreError(const std::string &msg) : _msg(msg) {}
        const char *what() const noexcept override { return _msg.c_str(); }
    };

    explicit Core(char **argv);
    ~Core();

    void run();
    static void launchMapEditor();

private:
    std::unique_ptr<MapEditor> _mapEditor;

    std::mutex _playerIdMutex;
    void parseCommandLineArgs(char **argv);
    void initializeNetwork();
    void setupNetworkCallbacks();
    void loadResources();

    void networkLoop();
    void graphicsLoop();

    void processOutgoingMessages();
    void processIncomingMessages(Window& window);
    void parseSnapshot(const std::vector<uint8_t>& payload);

    EntityType determineEntityType(uint32_t entityId, const std::string& spritePath);
    RenderLayer determineRenderLayer(EntityType type);

    void sendInput(KeyCode keyCode, InputAction action);
    void handleKeyStateChange(const std::string& action, bool isPressed, 
                             std::map<std::string, bool>& keyStates);
    void processInputs(Window& window, std::map<std::string, bool>& keyStates);

    std::unique_ptr<NetworkClient> _networkClient;
    std::string _hostname;
    int _port;
    std::string _username;
    uint8_t _myPlayerId;
    uint8_t _myPlayerEntityId;

    std::thread _networkThread;
    std::mutex _incomingMutex;
    std::mutex _outgoingMutex;
    std::mutex _snapshotMutex;
    std::queue<std::string> _incomingMessages;
    std::queue<std::string> _outgoingMessages;
    std::vector<uint8_t> _pendingSnapshot;
    bool _hasNewSnapshot;
    bool _running;

    std::unique_ptr<EntityManager> _entityManager;
    std::unique_ptr<ParallaxSystem> _parallaxSystem;
    std::unique_ptr<sf::Music> _backgroundMusic;
};

} // namespace CLIENT

int execute_rtypeClient(char **argv);