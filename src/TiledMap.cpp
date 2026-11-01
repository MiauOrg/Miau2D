#include "TiledMap.h"

#include <tmxlite/Layer.hpp>
#include <tmxlite/Tileset.hpp>
#include <tmxlite/ObjectGroup.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>

namespace {

bool HasFlag(std::uint8_t flags, std::uint8_t flag) {
    return (flags & flag) != 0;
}

}

bool TiledMap::Load(const std::string& path) {
    Unload();

    if (!map_.load(path)) {
        TraceLog(LOG_ERROR, "TMX: failed to load %s", path.c_str());
        return false;
    }

    if (map_.getOrientation() != tmx::Orientation::Orthogonal) {
        TraceLog(LOG_WARNING, "TMX: only orthogonal maps are supported");
    }

    for (const auto& tileset : map_.getTilesets()) {
        TilesetTexture entry{};
        entry.firstGID = tileset.getFirstGID();
        entry.tileCount = tileset.getTileCount();
        entry.columns = tileset.getColumnCount();
        entry.margin = static_cast<float>(tileset.getMargin());
        entry.spacing = static_cast<float>(tileset.getSpacing());
        entry.tileSize = {
            static_cast<float>(tileset.getTileSize().x),
            static_cast<float>(tileset.getTileSize().y)};

        entry.texture = LoadTexture(tileset.getImagePath().c_str());
        if (entry.texture.id == 0) {
            TraceLog(LOG_ERROR, "TMX: failed to load image %s", tileset.getImagePath().c_str());
        } else {
            SetTextureFilter(entry.texture, TEXTURE_FILTER_POINT);
        }

        tilesets_.push_back(entry);
    }

    loaded_ = true;
    return true;
}

void TiledMap::Unload() {
    for (auto& tileset : tilesets_) {
        if (tileset.texture.id != 0) {
            UnloadTexture(tileset.texture);
        }
    }

    tilesets_.clear();
    map_ = tmx::Map{};
    loaded_ = false;
}

void TiledMap::Draw() const {
    if (!loaded_) {
        return;
    }

    for (const auto& layer : map_.getLayers()) {
        if (!layer->getVisible() || layer->getType() != tmx::Layer::Type::Tile) {
            continue;
        }

        if (layer->getName() == "Collision") {
            continue;
        }

        DrawTileLayer(layer->getLayerAs<tmx::TileLayer>());
    }
}

Rectangle TiledMap::Bounds() const {
    const auto bounds = map_.getBounds();
    return {bounds.left, bounds.top, bounds.width, bounds.height};
}

Vector2 TiledMap::GetTileSize() const {
    const auto tileSize = map_.getTileSize();
    return {static_cast<float>(tileSize.x), static_cast<float>(tileSize.y)};
}

const TiledMap::TilesetTexture* TiledMap::FindTileset(std::uint32_t gid) const {
    for (const auto& tileset : tilesets_) {
        if (gid >= tileset.firstGID && gid < tileset.firstGID + tileset.tileCount) {
            return &tileset;
        }
    }

    return nullptr;
}

void TiledMap::DrawTileLayer(const tmx::TileLayer& layer) const {
    const auto& tiles = layer.getTiles();
    const auto size = layer.getSize();

    if (tiles.empty() || size.x == 0 || size.y == 0) {
        return;
    }

    for (std::size_t index = 0; index < tiles.size(); ++index) {
        const auto& tile = tiles[index];
        if (tile.ID == 0) {
            continue;
        }

        const TilesetTexture* tileset = FindTileset(tile.ID);
        if (tileset == nullptr || tileset->texture.id == 0 || tileset->columns == 0) {
            continue;
        }

        const std::uint32_t local = tile.ID - tileset->firstGID;
        const std::uint32_t column = local % tileset->columns;
        const std::uint32_t row = local / tileset->columns;

        Rectangle source{
            tileset->margin + static_cast<float>(column) * (tileset->tileSize.x + tileset->spacing),
            tileset->margin + static_cast<float>(row) * (tileset->tileSize.y + tileset->spacing),
            tileset->tileSize.x,
            tileset->tileSize.y};

        if (HasFlag(tile.flipFlags, tmx::TileLayer::Horizontal)) {
            source.x += source.width;
            source.width = -source.width;
        }
        if (HasFlag(tile.flipFlags, tmx::TileLayer::Vertical)) {
            source.y += source.height;
            source.height = -source.height;
        }

        const Rectangle destination{
            static_cast<float>(index % size.x) * tileset->tileSize.x,
            static_cast<float>(index / size.x) * tileset->tileSize.y,
            tileset->tileSize.x,
            tileset->tileSize.y};

        DrawTexturePro(tileset->texture, source, destination, {0.0f, 0.0f}, 0.0f,
                       Fade(WHITE, layer.getOpacity()));
    }
}

std::vector<AABB> TiledMap::GetLayerColliders(const std::string& layerName) const {
    std::vector<AABB> colliders;
    if (!loaded_) {
        return colliders;
    }

    const auto tileSize = map_.getTileSize();

    for (const auto& layer : map_.getLayers()) {
        if (layer->getName() != layerName) {
            continue;
        }

        if (layer->getType() == tmx::Layer::Type::Object) {
            const auto& objectGroup = layer->getLayerAs<tmx::ObjectGroup>();
            for (const auto& obj : objectGroup.getObjects()) {
                const auto& aabb = obj.getAABB();
                colliders.emplace_back(aabb.left, aabb.top, aabb.width, aabb.height);
            }
        } else if (layer->getType() == tmx::Layer::Type::Tile) {
            const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
            const auto& tiles = tileLayer.getTiles();
            const auto size = tileLayer.getSize();

            if (tiles.empty() || size.x == 0 || size.y == 0) {
                continue;
            }

            // Merge contiguous horizontal tiles into single AABB boxes
            for (std::uint32_t y = 0; y < size.y; ++y) {
                std::uint32_t startX = 0;
                bool inSpan = false;

                for (std::uint32_t x = 0; x < size.x; ++x) {
                    const auto& tile = tiles[y * size.x + x];
                    const bool isSolid = (tile.ID != 0);

                    if (isSolid) {
                        if (!inSpan) {
                            startX = x;
                            inSpan = true;
                        }
                    } else {
                        if (inSpan) {
                            colliders.emplace_back(
                                static_cast<float>(startX * tileSize.x),
                                static_cast<float>(y * tileSize.y),
                                static_cast<float>((x - startX) * tileSize.x),
                                static_cast<float>(tileSize.y)
                            );
                            inSpan = false;
                        }
                    }
                }

                if (inSpan) {
                    colliders.emplace_back(
                        static_cast<float>(startX * tileSize.x),
                        static_cast<float>(y * tileSize.y),
                        static_cast<float>((size.x - startX) * tileSize.x),
                        static_cast<float>(tileSize.y)
                    );
                }
            }
        }
    }

    return colliders;
}

std::vector<AABB> TiledMap::GetColliders() const {
    std::vector<AABB> colliders;
    if (!loaded_) {
        return colliders;
    }

    for (const auto& layer : map_.getLayers()) {
        if (layer->getType() == tmx::Layer::Type::Object) {
            const auto layerColliders = GetLayerColliders(layer->getName());
            colliders.insert(colliders.end(), layerColliders.begin(), layerColliders.end());
            continue;
        }

        std::string nameLower = layer->getName();
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (nameLower.find("collision") != std::string::npos ||
            nameLower.find("solid") != std::string::npos ||
            nameLower.find("obstacle") != std::string::npos) {
            const auto layerColliders = GetLayerColliders(layer->getName());
            colliders.insert(colliders.end(), layerColliders.begin(), layerColliders.end());
        }
    }

    return colliders;
}
