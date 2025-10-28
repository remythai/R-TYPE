/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.hpp
*/

#pragma once

#include <SFML/Audio.hpp>
#include <chrono>
#include <cmath>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>

#include "../colorBlindFilter/ColorBlindFilter.hpp"
#include "../entityManager/EntityManager.hpp"
#include "../keybind/KeybindManager.hpp"
#include "../keybind/KeybindMenu.hpp"
#include "../mapEditor/MapEditor.hpp"
#include "../network/NetworkClient.hpp"
#include "../parallax/ParallaxSystem.hpp"

namespace CLIENT {

class Window;

enum class KeyCode : uint8_t
{
    DOWN = 0,
    UP = 1,
    LEFT = 2,
    RIGHT = 3,
    SHOOT = 4
};

enum class InputAction : uint8_t
{
    PRESSED = 1,
    RELEASED = 0
};

class Core
{
   public:
    class CoreError : public std::exception
    {
       private:
        std::string _msg;

       public:
        explicit CoreError(const std::string& msg) : _msg(msg) {}
        const char* what() const noexcept override
        {
            return _msg.c_str();
        }
    };

    explicit Core(char** argv);
    ~Core();

    void run();
    static void launchMapEditor();

   private:
    enum class GameState
    {
        PLAYING,
        DEFEATED,
        DISCONNECTED
    };

    void handleTimeoutEvent(uint8_t playerId);
    void handleKilledEvent(uint8_t playerId);
    void loadDefeatScreen();
    void renderDefeatScreen(sf::RenderTarget& target);

    GameState _gameState;
    sf::Texture _defeatTexture;
    std::optional<sf::Sprite> _defeatSprite;
    bool _defeatTextureLoaded;

    std::unique_ptr<MapEditor> _mapEditor;

    std::mutex _playerIdMutex;

    void parseCommandLineArgs(char** argv);
    void initializeNetwork();
    void setupNetworkCallbacks();
    void loadResources();
    void loadGameTextures();
    void loadParallaxTextures();
    void loadBackgroundMusic();

    void handlePlayerIdReceived(uint8_t playerId);
    void handlePlayerEvent(uint8_t playerId, uint8_t eventType);
    void handleSnapshotReceived(const std::vector<uint8_t>& payload);

    void networkLoop();
    void graphicsLoop();

    void initializeGraphicsComponents();
    void updateFromSnapshot();
    bool shouldCleanupEntities(
        const std::chrono::steady_clock::time_point& lastCleanup,
        float interval);
    void renderFrame(Window& window, float deltaTime);

    void processOutgoingMessages();
    void sendInputMessage(const std::string& msg);
    void processIncomingMessages(Window& window);
    void handleIncomingMessage(const std::string& msg, Window& window);
    void handlePlayerLeave(const std::string& msg, Window& window);

    void parseSnapshot(const std::vector<uint8_t>& payload);
    float readFloat(const std::vector<uint8_t>& payload, size_t& offset);
    bool parseSnapshotEntity(
        const std::vector<uint8_t>& payload, size_t& offset,
        std::set<uint8_t>& activeEntities);
    void updateOrCreateEntity(
        uint8_t entityId, float x, float y, const std::string& spritePath,
        float rectPosX, float rectPosY, float rectSizeX, float rectSizeY);
    void updateEntityPosition(GameEntity* entity, float x, float y);
    void updateEntitySprite(
        GameEntity* entity, uint8_t entityId, const std::string& spritePath,
        bool needsNewSprite, float rectPosX, float rectPosY, float rectSizeX,
        float rectSizeY);
    sf::Texture* findTexture(const std::string& spritePath, uint8_t entityId);
    void applySpriteTransform(
        sf::Sprite& sprite, float rectPosX, float rectPosY, float rectSizeX,
        float rectSizeY, const sf::Vector2f& position);

    void sendInput(KeyCode keyCode, InputAction action);
    void handleKeyStateChange(
        const std::string& action, bool isPressed,
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

    std::unique_ptr<KeybindManager> _keybindManager;
    std::unique_ptr<KeybindMenu> _keybindMenu;

    std::unique_ptr<ColorBlindFilter> _colorBlindFilter;
};

}  // namespace CLIENT

int execute_rtypeClient(char** argv);