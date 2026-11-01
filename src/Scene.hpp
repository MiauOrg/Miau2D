#pragma once

#include "Physics/PhysicsWorld.hpp"
#include "TiledMap.h"

#include <raylib.h>

#include <memory>
#include <vector>

class GameObject;
class Player;

class Scene {
public:
    Scene();
    ~Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    bool Load();
    void Unload();

    void Update(float deltaTime);
    void Draw();

    [[nodiscard]] PhysicsWorld& GetPhysics() { return physics_; }
    [[nodiscard]] const PhysicsWorld& GetPhysics() const { return physics_; }
    [[nodiscard]] const Player* GetPlayer() const { return player_; }

    [[nodiscard]] bool IsDebugVisible() const { return debugVisible_; }
    void ToggleDebug() { debugVisible_ = !debugVisible_; }

private:
    TiledMap map_;
    PhysicsWorld physics_;
    Camera2D camera_{};

    std::vector<std::unique_ptr<GameObject>> objects_;
    std::vector<GameObject*> drawList_;
    Player* player_ = nullptr;

    Texture2D playerTexture_{};
    bool debugVisible_ = false;
    bool loaded_ = false;
};
