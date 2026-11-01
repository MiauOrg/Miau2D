#pragma once

#include <raylib.h>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>

#include "Physics/AABB.hpp"
#include "Physics/Circle.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class TiledMap {
public:
    bool Load(const std::string& path);
    void Unload();
    void Draw() const;
    void DrawDepthTilesBefore(float sortY, std::size_t& nextTile) const;
    void DrawRemainingDepthTiles(std::size_t nextTile) const;

    [[nodiscard]] bool IsLoaded() const { return loaded_; }
    [[nodiscard]] Rectangle Bounds() const;
    [[nodiscard]] Vector2 GetTileSize() const;

    [[nodiscard]] std::vector<AABB> GetColliders() const;
    [[nodiscard]] std::vector<AABB> GetLayerColliders(const std::string& layerName) const;
    [[nodiscard]] std::vector<Circle> GetCircleColliders() const;

private:
    struct TilesetTexture {
        Texture2D texture{};
        std::uint32_t firstGID = 0;
        std::uint32_t tileCount = 0;
        std::uint32_t columns = 0;
        float margin = 0.0f;
        float spacing = 0.0f;
        Vector2 tileSize{};
        std::vector<std::optional<float>> sortOffsets;
    };

    struct TileDraw {
        Texture2D texture{};
        Rectangle source{};
        Rectangle destination{};
        Color tint = WHITE;
        float sortY = 0.0f;
    };

    const TilesetTexture* FindTileset(std::uint32_t gid) const;
    void BuildDrawTiles();
    void DrawTile(const TileDraw& tile) const;

    tmx::Map map_;
    std::vector<TilesetTexture> tilesets_;
    std::vector<TileDraw> backgroundTiles_;
    std::vector<TileDraw> depthTiles_;
    bool loaded_ = false;
};
