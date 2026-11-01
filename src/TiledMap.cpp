#include "TiledMap.hpp"

#include <tmxlite/Layer.hpp>
#include <tmxlite/Tileset.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/Property.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <string>

namespace {

bool HasFlag(std::uint8_t flags, std::uint8_t flag) {
    return (flags & flag) != 0;
}

bool HasBoolProperty(const tmx::Layer& layer, const std::string& name) {
    for (const auto& property : layer.getProperties()) {
        if (property.getName() == name &&
            property.getType() == tmx::Property::Type::Boolean) {
            return property.getBoolValue();
        }
    }
    return false;
}

bool NameContains(const tmx::Layer& layer, const char* needle) {
    std::string name = layer.getName();
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return name.find(needle) != std::string::npos;
}

bool IsAboveLayer(const tmx::Layer& layer) {
    return HasBoolProperty(layer, "above") ||
           NameContains(layer, "overhead") ||
           NameContains(layer, "above");
}

bool IsYSortLayer(const tmx::Layer& layer) {
    return !IsAboveLayer(layer) &&
           (HasBoolProperty(layer, "ysort") ||
            NameContains(layer, "ysort") ||
            NameContains(layer, "props"));
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
        entry.sortOffsets.resize(entry.tileCount);
        for (const auto& tile : tileset.getTiles()) {
            if (tile.ID >= entry.sortOffsets.size()) {
                continue;
            }
            for (const auto& property : tile.properties) {
                if (property.getName() != "sortOffsetY") {
                    continue;
                }
                float offset = 0.0f;
                if (property.getType() == tmx::Property::Type::Float) {
                    offset = property.getFloatValue();
                } else if (property.getType() == tmx::Property::Type::Int) {
                    offset = static_cast<float>(property.getIntValue());
                } else {
                    continue;
                }
                if (std::isfinite(offset)) {
                    entry.sortOffsets[tile.ID] = offset;
                }
            }
        }

        entry.texture = LoadTexture(tileset.getImagePath().c_str());
        if (entry.texture.id == 0) {
            TraceLog(LOG_ERROR, "TMX: failed to load image %s", tileset.getImagePath().c_str());
        } else {
            SetTextureFilter(entry.texture, TEXTURE_FILTER_POINT);
        }

        tilesets_.push_back(entry);
    }

    loaded_ = true;
    BuildSortedTiles();
    return true;
}

void TiledMap::Unload() {
    belowTiles_.clear();
    sortedTiles_.clear();
    aboveTiles_.clear();

    for (auto& tileset : tilesets_) {
        if (tileset.texture.id != 0) {
            UnloadTexture(tileset.texture);
        }
    }

    tilesets_.clear();
    map_ = tmx::Map{};
    loaded_ = false;
}

void TiledMap::DrawLayerPass(LayerPass pass) const {
    const auto& tiles = pass == LayerPass::Above ? aboveTiles_ : belowTiles_;
    for (const TileQuad& tile : tiles) {
        tile.Draw();
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

bool TiledMap::BuildTileQuad(const tmx::TileLayer& layer, std::size_t index, TileQuad& out) const {
    const auto& tiles = layer.getTiles();
    const auto size = layer.getSize();

    if (tiles.empty() || size.x == 0 || size.y == 0 || index >= tiles.size()) {
        return false;
    }

    const auto& tile = tiles[index];
    if (tile.ID == 0) {
        return false;
    }

    const TilesetTexture* tileset = FindTileset(tile.ID);
    if (tileset == nullptr || tileset->texture.id == 0 || tileset->columns == 0) {
        return false;
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

    const Vector2 mapTileSize = GetTileSize();
    const Rectangle destination{
        static_cast<float>(index % size.x) * mapTileSize.x + layer.getOffset().x,
        static_cast<float>(index / size.x) * mapTileSize.y + layer.getOffset().y,
        tileset->tileSize.x,
        tileset->tileSize.y};

    out.texture = tileset->texture;
    out.source = source;
    out.destination = destination;
    out.tint = Fade(WHITE, layer.getOpacity());
    out.sortY = destination.y + destination.height + tileset->sortOffsets[local].value_or(0.0f);
    return true;
}

void TiledMap::BuildSortedTiles() {
    belowTiles_.clear();
    sortedTiles_.clear();
    aboveTiles_.clear();

    for (const auto& layer : map_.getLayers()) {
        if (!layer->getVisible() || layer->getType() != tmx::Layer::Type::Tile) {
            continue;
        }

        if (layer->getName() == "Collision") {
            continue;
        }

        const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
        const auto& tiles = tileLayer.getTiles();
        const bool above = IsAboveLayer(*layer);
        const bool ySorted = IsYSortLayer(*layer);

        for (std::size_t index = 0; index < tiles.size(); ++index) {
            TileQuad quad;
            if (!BuildTileQuad(tileLayer, index, quad)) {
                continue;
            }
            const TilesetTexture* tileset = FindTileset(tiles[index].ID);
            const bool hasSortOffset = tileset != nullptr &&
                tileset->sortOffsets[tiles[index].ID - tileset->firstGID].has_value();
            if (above) {
                aboveTiles_.push_back(quad);
            } else if (ySorted || hasSortOffset) {
                sortedTiles_.push_back(quad);
            } else {
                belowTiles_.push_back(quad);
            }
        }
    }

    std::stable_sort(sortedTiles_.begin(), sortedTiles_.end(),
                     [](const TileQuad& a, const TileQuad& b) {
                         return a.sortY < b.sortY;
                     });
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
            if (layer->getName() == "CircleCollision") {
                continue;
            }
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

std::vector<Circle> TiledMap::GetCircleColliders() const {
    std::vector<Circle> colliders;
    if (!loaded_) {
        return colliders;
    }

    for (const auto& layer : map_.getLayers()) {
        if (layer->getName() != "CircleCollision" || layer->getType() != tmx::Layer::Type::Object) {
            continue;
        }

        const auto& objectGroup = layer->getLayerAs<tmx::ObjectGroup>();
        for (const auto& object : objectGroup.getObjects()) {
            if (object.getShape() != tmx::Object::Shape::Ellipse) {
                continue;
            }

            const auto& bounds = object.getAABB();
            if (bounds.width <= 0.0f || bounds.height <= 0.0f ||
                std::fabs(bounds.width - bounds.height) > 0.001f) {
                continue;
            }

            colliders.push_back({
                { bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f },
                bounds.width * 0.5f
            });
        }
    }

    return colliders;
}
