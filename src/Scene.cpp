#include "Scene.hpp"

#include "Actor.hpp"
#include "AppConfig.hpp"
#include "GameObject.hpp"
#include "Movement.hpp"
#include "Player.hpp"

#include <raymath.h>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {

constexpr Vector2 PlayerStartCell { 13.0f, 8.0f };
constexpr float PlayerSpriteSize = 24.0f;

Vector2 CameraFocus(const Player& player) {
    return player.transform.position - Vector2 { 0.0f, player.frame.height * 0.5f };
}

}  // namespace

Scene::Scene() = default;

Scene::~Scene() {
    Unload();
}

bool Scene::Load() {
    Unload();

    if (!map_.Load("tiled/Overworld01.tmx")) {
        return false;
    }

    physics_.ClearStaticColliders();
    physics_.ClearWorldBounds();
    physics_.SetWorldBounds(map_.Bounds());
    physics_.AddStaticColliders(map_.GetColliders());

    playerTexture_ = LoadTexture("assets/characters/Character.png");
    if (playerTexture_.id != 0) {
        SetTextureFilter(playerTexture_, TEXTURE_FILTER_POINT);
    }

    auto player = std::make_unique<Player>();
    player->texture = playerTexture_;
    player->frame = { 0.0f, 0.0f, PlayerSpriteSize, PlayerSpriteSize };
    player->collider.size = { 16.0f, 16.0f };
    Movement::SetTileSize(*player, map_.GetTileSize());
    player->transform.position = player->CellToPosition(PlayerStartCell);
    Movement::SnapToGrid(*player, &physics_);

    player_ = player.get();
    objects_.push_back(std::move(player));

    camera_ = Camera2D{};
    camera_.zoom = 1.0f;
    camera_.target = CameraFocus(*player_) - Vector2 {
        VirtualWidth * 0.5f,
        VirtualHeight * 0.5f
    };

    loaded_ = true;
    return true;
}

void Scene::Unload() {
    objects_.clear();
    drawList_.clear();
    player_ = nullptr;

    if (playerTexture_.id != 0) {
        UnloadTexture(playerTexture_);
        playerTexture_ = Texture2D{};
    }

    map_.Unload();
    physics_.ClearStaticColliders();
    physics_.ClearWorldBounds();
    loaded_ = false;
}

void Scene::Update(float deltaTime) {
    if (!loaded_) {
        return;
    }

    if (IsKeyDown(KEY_EQUAL) || IsKeyDown(KEY_KP_ADD)) {
        camera_.zoom += 2.0f * deltaTime;
    }
    if (IsKeyDown(KEY_MINUS) || IsKeyDown(KEY_KP_SUBTRACT)) {
        camera_.zoom -= 2.0f * deltaTime;
    }
    camera_.zoom = Clamp(camera_.zoom, 0.25f, 4.0f);

    for (auto& object : objects_) {
        if (object->active) {
            object->Update(*this, deltaTime);
        }
    }

    if (player_ != nullptr) {
        const Vector2 target = CameraFocus(*player_) - Vector2 {
            VirtualWidth * 0.5f,
            VirtualHeight * 0.5f
        };
        camera_.target = Vector2Lerp(camera_.target, target, deltaTime * 5.0f);
    }
}

void Scene::Draw() {
    if (!loaded_) {
        return;
    }

    // Snap the camera to whole pixels to avoid jitter in the low-res target.
    Camera2D renderCamera = camera_;
    renderCamera.target.x = std::floor(renderCamera.target.x);
    renderCamera.target.y = std::floor(renderCamera.target.y);

    BeginMode2D(renderCamera);

    map_.DrawLayerPass(TiledMap::LayerPass::Below);

    const Rectangle bounds = map_.Bounds();
    const Vector2 tileSize = map_.GetTileSize();

    if (debugVisible_ && tileSize.x > 0.0f && tileSize.y > 0.0f) {
        for (float x = bounds.x; x <= bounds.x + bounds.width + 0.5f; x += tileSize.x) {
            DrawLineV({ x, bounds.y }, { x, bounds.y + bounds.height }, Fade(WHITE, 0.12f));
        }
        for (float y = bounds.y; y <= bounds.y + bounds.height + 0.5f; y += tileSize.y) {
            DrawLineV({ bounds.x, y }, { bounds.x + bounds.width, y }, Fade(WHITE, 0.12f));
        }
    }

    // Collect drawable objects and y-sort them so actors overlap correctly.
    drawList_.clear();
    for (auto& object : objects_) {
        if (object->active && object->IsVisible()) {
            drawList_.push_back(object.get());
        }
    }
    std::stable_sort(drawList_.begin(), drawList_.end(),
                     [](const GameObject* a, const GameObject* b) {
                         return a->SortY() < b->SortY();
                     });

    const auto& mapTiles = map_.SortedTiles();
    std::size_t tileIndex = 0;

    for (const auto* object : drawList_) {
        while (tileIndex < mapTiles.size() &&
               mapTiles[tileIndex].sortY <= object->SortY()) {
            mapTiles[tileIndex].Draw();
            ++tileIndex;
        }
        object->Draw();
    }
    while (tileIndex < mapTiles.size()) {
        mapTiles[tileIndex].Draw();
        ++tileIndex;
    }

    map_.DrawLayerPass(TiledMap::LayerPass::Above);

    if (debugVisible_) {
        physics_.DrawDebug();
        for (auto& object : objects_) {
            object->DrawDebug();
        }
    }

    EndMode2D();
}
