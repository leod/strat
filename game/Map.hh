#ifndef STRAT_GAME_MAP_HH
#define STRAT_GAME_MAP_HH

#include <entityx/entityx.h>

#include <cstring>
#include <cassert>
#include <vector>

struct GridPoint {
    size_t height;

    entityx::Entity entity;

    GridPoint()
        : height(0), entity() {
    }
};

struct Map {
    Map(size_t sizeX, size_t sizeY);

    size_t getSizeX() const { return sizeX; }
    size_t getSizeY() const { return sizeY; }

    size_t getMaxHeight() const { return maxHeight; }

    GridPoint &point(size_t x, size_t y) {
        assert(x < sizeX);
        assert(y < sizeY);
        return points[y * sizeX + x];
    }

    const GridPoint &point(size_t x, size_t y) const {
        assert(x < sizeX);
        assert(y < sizeY);
        return points[y * sizeX + x];
    }

    static Map *generate(size_t sizeX, size_t sizeY, size_t heightLimit);

private:
    size_t sizeX;
    size_t sizeY;

    size_t maxHeight;

    std::vector<GridPoint> points; // 2d array
};

#endif
