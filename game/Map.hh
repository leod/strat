#ifndef STRAT_GAME_MAP_HH
#define STRAT_GAME_MAP_HH

#include <entityx/entityx.h>
#include <glm/glm.hpp>
#include <Fixed.hh>

#include <cstring>
#include <cassert>
#include <vector>

struct GridPoint {
    size_t height;

    entityx::Entity entity;

    Fixed water;

    GridPoint()
        : height(0), entity(), water(0) {
    }
};

struct Map {
    typedef glm::uvec2 Pos;

    Map(size_t sizeX, size_t sizeY);

    size_t getSizeX() const { return sizeX; }
    size_t getSizeY() const { return sizeY; }

    Pos getSize() const {
        return Pos(sizeX, sizeY);
    }

    size_t getMaxHeight() const { return maxHeight; }

    bool isPoint(const Pos &p) const {
        return p.x < sizeX && p.y < sizeY;
    }

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

    GridPoint &point(const Pos &p) {
        return point(p.x, p.y);
    }

    const GridPoint &point(const Pos &p) const {
        return point(p.x, p.y);
    }

    static Map generate(size_t sizeX, size_t sizeY,
                        size_t heightLimit, size_t seed);

    template<typename F>
    void forNeighbors(const Pos &p, F f) {
        assert(isPoint(p));

        if (p.y > 0) f(Pos(p.x, p.y-1));
        if (p.x < sizeX-1 && p.y > 0) f(Pos(p.x+1, p.y-1));
        if (p.x < sizeX-1 && p.y < sizeY-1) f(Pos(p.x+1, p.y+1));
        if (p.x > 0 && p.y < sizeY-1) f(Pos(p.x-1, p.y+1));
    }

private:
    size_t sizeX;
    size_t sizeY;

    size_t maxHeight;

    std::vector<GridPoint> points; // 2d array
};

#endif
