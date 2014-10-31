#ifndef STRAT_GAME_MAP_HH
#define STRAT_GAME_MAP_HH

#include <entityx/entityx.h>
#include <glm/glm.hpp>

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

    glm::uvec2 getSize() const {
        return glm::uvec2(sizeX, sizeY);
    }

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

    GridPoint &point(const glm::uvec2 &p) {
        return point(p.x, p.y);
    }

    const GridPoint &point(const glm::uvec2 &p) const {
        return point(p.x, p.y);
    }

    static Map generate(size_t sizeX, size_t sizeY,
                        size_t heightLimit, size_t seed);

private:
    size_t sizeX;
    size_t sizeY;

    size_t maxHeight;

    std::vector<GridPoint> points; // 2d array
};

#endif
