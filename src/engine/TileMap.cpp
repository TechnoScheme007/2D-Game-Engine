#include "engine/TileMap.h"
#include <algorithm>

namespace Engine {

void TileMap::init(int mapWidth, int mapHeight, int tileSize, const std::string& tilesetId) {
    mapWidth_ = mapWidth;
    mapHeight_ = mapHeight;
    tileSize_ = tileSize;
    tilesetId_ = tilesetId;

    tiles_.resize(mapHeight);
    for (auto& row : tiles_) {
        row.resize(mapWidth);
    }
}

void TileMap::setTile(int x, int y, int tileId, bool solid, int srcX, int srcY) {
    if (x < 0 || x >= mapWidth_ || y < 0 || y >= mapHeight_) return;
    tiles_[y][x] = {tileId, solid, srcX, srcY};
}

Tile TileMap::getTile(int x, int y) const {
    if (x < 0 || x >= mapWidth_ || y < 0 || y >= mapHeight_) return {-1, true, 0, 0};
    return tiles_[y][x];
}

bool TileMap::isSolid(int worldX, int worldY) const {
    int tx = worldX / tileSize_;
    int ty = worldY / tileSize_;
    return getTile(tx, ty).solid;
}

void TileMap::render(Renderer& renderer) const {
    Vec2 cam = renderer.getCamera();
    int startX = std::max(0, static_cast<int>(cam.x) / tileSize_);
    int startY = std::max(0, static_cast<int>(cam.y) / tileSize_);
    int endX = std::min(mapWidth_, static_cast<int>(cam.x + renderer.getWidth()) / tileSize_ + 2);
    int endY = std::min(mapHeight_, static_cast<int>(cam.y + renderer.getHeight()) / tileSize_ + 2);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            const Tile& tile = tiles_[y][x];
            if (tile.id < 0) continue;

            SDL_Rect src = {tile.srcX, tile.srcY, tileSize_, tileSize_};
            SDL_Rect dst = {x * tileSize_, y * tileSize_, tileSize_, tileSize_};
            renderer.drawTexture(tilesetId_, src, dst);
        }
    }
}

bool TileMap::checkCollision(const Rect& aabb) const {
    int startX = std::max(0, static_cast<int>(aabb.x) / tileSize_);
    int startY = std::max(0, static_cast<int>(aabb.y) / tileSize_);
    int endX = std::min(mapWidth_ - 1, static_cast<int>(aabb.x + aabb.w) / tileSize_);
    int endY = std::min(mapHeight_ - 1, static_cast<int>(aabb.y + aabb.h) / tileSize_);

    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            if (tiles_[y][x].solid) return true;
        }
    }
    return false;
}

Vec2 TileMap::resolveCollision(const Vec2& pos, const Vec2& size, const Vec2& velocity) const {
    Vec2 newPos = pos;

    // Resolve X first
    if (velocity.x != 0) {
        Vec2 testPos = {pos.x + velocity.x, pos.y};
        Rect testRect = {testPos.x, testPos.y, size.x, size.y};
        if (!checkCollision(testRect)) {
            newPos.x = testPos.x;
        } else {
            // Snap to tile boundary
            if (velocity.x > 0) {
                int tileEdge = (static_cast<int>(testPos.x + size.x) / tileSize_) * tileSize_;
                newPos.x = tileEdge - size.x - 0.01f;
            } else {
                int tileEdge = (static_cast<int>(testPos.x) / tileSize_ + 1) * tileSize_;
                newPos.x = static_cast<float>(tileEdge) + 0.01f;
            }
        }
    }

    // Then resolve Y
    if (velocity.y != 0) {
        Vec2 testPos = {newPos.x, pos.y + velocity.y};
        Rect testRect = {testPos.x, testPos.y, size.x, size.y};
        if (!checkCollision(testRect)) {
            newPos.y = testPos.y;
        } else {
            if (velocity.y > 0) {
                int tileEdge = (static_cast<int>(testPos.y + size.y) / tileSize_) * tileSize_;
                newPos.y = tileEdge - size.y - 0.01f;
            } else {
                int tileEdge = (static_cast<int>(testPos.y) / tileSize_ + 1) * tileSize_;
                newPos.y = static_cast<float>(tileEdge) + 0.01f;
            }
        }
    }

    return newPos;
}

void TileMap::loadFromArray(const std::vector<std::vector<int>>& data,
                             const std::vector<bool>& solidTiles,
                             int tilesetCols) {
    for (int y = 0; y < mapHeight_ && y < static_cast<int>(data.size()); y++) {
        for (int x = 0; x < mapWidth_ && x < static_cast<int>(data[y].size()); x++) {
            int id = data[y][x];
            bool solid = (id >= 0 && id < static_cast<int>(solidTiles.size())) ? solidTiles[id] : false;
            int srcX = (id % tilesetCols) * tileSize_;
            int srcY = (id / tilesetCols) * tileSize_;
            setTile(x, y, id, solid, srcX, srcY);
        }
    }
}

} // namespace Engine
