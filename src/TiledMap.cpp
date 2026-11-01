#include "TiledMap.hpp"

#include <tmxlite/Layer.hpp>
#include <tmxlite/Tileset.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/Property.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cmath>

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

    BuildDrawTiles();
    loaded_ = true;
    return true;
}

void TiledMap::Unload() {
    backgroundTiles_.clear();
    depthTiles_.clear();
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
    for (const TileDraw& tile : backgroundTiles_) {
        DrawTile(tile);
    }
}

void TiledMap::DrawDepthTilesBefore(float sortY, std::size_t& nextTile) const {
    while (nextTile < depthTiles_.size() && depthTiles_[nextTile].sortY <= sortY) {
        DrawTile(depthTiles_[nextTile]);
        ++nextTile;
    }
}

void TiledMap::DrawRemainingDepthTiles(std::size_t nextTile) const {
    for (; nextTile < depthTiles_.size(); ++nextTile) {
        DrawTile(depthTiles_[nextTile]);
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

void TiledMap::BuildDrawTiles() {
    backgroundTiles_.clear();
    depthTiles_.clear();
    const Vector2 mapTileSize = GetTileSize();

    for (const auto& layer : map_.getLayers()) {
        if (!layer->getVisible() || layer->getType() != tmx::Layer::Type::Tile ||
            layer->getName() == "Collision") {
            continue;
        }
        const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
        const auto size = tileLayer.getSize();
        if (size.x == 0) {
            continue;
        }
        const auto& tiles = tileLayer.getTiles();
        for (std::size_t index = 0; index < tiles.size(); ++index) {
            const auto& tile = tiles[index];
            const TilesetTexture* tileset = FindTileset(tile.ID);
            if (tileset == nullptr || tileset->texture.id == 0 || tileset->columns == 0) {
                continue;
            }
            const std::uint32_t local = tile.ID - tileset->firstGID;
            const std::uint32_t column = local % tileset->columns;
            const std::uint32_t atlasRow = local / tileset->columns;

            TileDraw draw{};
            draw.texture = tileset->texture;
            draw.source = {
                tileset->margin + static_cast<float>(column) * (tileset->tileSize.x + tileset->spacing),
                tileset->margin + static_cast<float>(atlasRow) * (tileset->tileSize.y + tileset->spacing),
                tileset->tileSize.x,
                tileset->tileSize.y
            };
            if (HasFlag(tile.flipFlags, tmx::TileLayer::Horizontal)) {
                draw.source.x += draw.source.width;
                draw.source.width = -draw.source.width;
            }
            if (HasFlag(tile.flipFlags, tmx::TileLayer::Vertical)) {
                draw.source.y += draw.source.height;
                draw.source.height = -draw.source.height;
            }
            draw.destination = {
                static_cast<float>(index % size.x) * mapTileSize.x + layer->getOffset().x,
                static_cast<float>(index / size.x) * mapTileSize.y + layer->getOffset().y,
                tileset->tileSize.x,
                tileset->tileSize.y
            };
            draw.tint = Fade(WHITE, layer->getOpacity());
            const auto sortOffset = tileset->sortOffsets[local];
            if (sortOffset.has_value()) {
                draw.sortY = draw.destination.y + draw.destination.height + *sortOffset;
                depthTiles_.push_back(draw);
            } else {
                backgroundTiles_.push_back(draw);
            }
        }
    }

    std::stable_sort(depthTiles_.begin(), depthTiles_.end(),
        [](const TileDraw& a, const TileDraw& b) {
            return a.sortY < b.sortY;
        });
}

void TiledMap::DrawTile(const TileDraw& tile) const {
    DrawTexturePro(tile.texture, tile.source, tile.destination, { 0.0f, 0.0f }, 0.0f, tile.tint);
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
