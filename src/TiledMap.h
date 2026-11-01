#pragma once

#include <raylib.h>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>

#include "Physics/AABB.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class TiledMap {
public:
    enum class LayerPass {
        Below,
        Above,
    };

    // A pre-baked tile quad from a y-sorted layer. The list is static and
    // sorted once at load time, then merged with actors every frame.
    struct TileQuad {
        Texture2D texture{};
        Rectangle source{};
        Rectangle destination{};
        Color tint = WHITE;
        float sortY = 0.0f;

        void Draw() const {
            DrawTexturePro(texture, source, destination, { 0.0f, 0.0f }, 0.0f, tint);
        }
    };

    bool Load(const std::string& path);
    void Unload();

    // Draws normal tile layers. y-sorted layers are drawn by the caller via
    // SortedTiles() so they can be interleaved with actors.
    void DrawLayerPass(LayerPass pass) const;

    [[nodiscard]] bool IsLoaded() const { return loaded_; }
    [[nodiscard]] Rectangle Bounds() const;
    [[nodiscard]] Vector2 GetTileSize() const;

    [[nodiscard]] const std::vector<TileQuad>& SortedTiles() const { return sortedTiles_; }

    [[nodiscard]] std::vector<AABB> GetColliders() const;
    [[nodiscard]] std::vector<AABB> GetLayerColliders(const std::string& layerName) const;

private:
    struct TilesetTexture {
        Texture2D texture{};
        std::uint32_t firstGID = 0;
        std::uint32_t tileCount = 0;
        std::uint32_t columns = 0;
        float margin = 0.0f;
        float spacing = 0.0f;
        Vector2 tileSize{};
    };

    const TilesetTexture* FindTileset(std::uint32_t gid) const;
    bool BuildTileQuad(const tmx::TileLayer& layer, std::size_t index, TileQuad& out) const;
    void DrawTileLayer(const tmx::TileLayer& layer) const;
    void BuildSortedTiles();

    tmx::Map map_;
    std::vector<TilesetTexture> tilesets_;
    std::vector<TileQuad> sortedTiles_;
    bool loaded_ = false;
};
