#pragma once

#include <raylib.h>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>

#include <cstdint>
#include <string>
#include <vector>

class TiledMap {
public:
    bool Load(const std::string& path);
    void Unload();
    void Draw() const;

    [[nodiscard]] bool IsLoaded() const { return loaded_; }
    [[nodiscard]] Rectangle Bounds() const;

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
    void DrawTileLayer(const tmx::TileLayer& layer) const;

    tmx::Map map_;
    std::vector<TilesetTexture> tilesets_;
    bool loaded_ = false;
};
