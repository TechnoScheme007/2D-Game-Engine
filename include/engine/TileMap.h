#pragma once
#include "Math.h"
#include "Renderer.h"
#include <vector>
#include <string>

namespace Engine {

struct Tile {
    int id = -1;          // -1 = empty
    bool solid = false;
    int srcX = 0, srcY = 0; // position in tileset texture
};

class TileMap {
public:
    TileMap() = default;

    void init(int mapWidth, int mapHeight, int tileSize, const std::string& tilesetId);

    void setTile(int x, int y, int tileId, bool solid = false, int srcX = 0, int srcY = 0);
    Tile getTile(int x, int y) const;
    bool isSolid(int worldX, int worldY) const;

    void render(Renderer& renderer) const;

    // Check AABB against solid tiles
    bool checkCollision(const Rect& aabb) const;
    // Resolve AABB collision with tiles, returns corrected position
    Vec2 resolveCollision(const Vec2& pos, const Vec2& size, const Vec2& velocity) const;

    int getMapWidth() const { return mapWidth_; }
    int getMapHeight() const { return mapHeight_; }
    int getTileSize() const { return tileSize_; }
    int getPixelWidth() const { return mapWidth_ * tileSize_; }
    int getPixelHeight() const { return mapHeight_ * tileSize_; }

    // Load from a simple format: comma-separated tile IDs
    void loadFromArray(const std::vector<std::vector<int>>& data,
                       const std::vector<bool>& solidTiles,
                       int tilesetCols);

private:
    std::vector<std::vector<Tile>> tiles_;
    std::string tilesetId_;
    int mapWidth_ = 0;
    int mapHeight_ = 0;
    int tileSize_ = 32;
};

} // namespace Engine
