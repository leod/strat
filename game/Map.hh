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

    // Progress of heightening
    size_t growthTarget;
    Fixed growthProgress; // 0 <= growthProgress <= growthTarget
    bool growthCascade;

    entityx::Entity entity;

    Fixed water;

    GridPoint()
        : height(0),
          growthTarget(0), growthProgress(0),
          growthCascade(false),
          entity(),
          water(0) {
    }

    bool usable() const {
        assert(growthTarget != 0 || growthProgress == Fixed(0));

        return growthTarget == 0 &&
               !entity &&
               water == Fixed(0);
    }
};

// The map holds all the information about the terrain
// that is needed for the game logic simulation.
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

    bool isPoint(size_t x, size_t y) const {
        return isPoint(Map::Pos(x, y));
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

        if (p.y > 0) f(point(p.x, p.y-1));
        if (p.y < sizeY - 1) f(point(p.x, p.y+1));
        if (p.x > 0) f(point(p.x-1, p.y));
        if (p.x < sizeX - 1) f(point(p.x+1, p.y));
        if (p.x > 0 && p.y > 0) f(point(p.x-1, p.y-1));
        if (p.x > 0 && p.y < sizeY-1) f(point(p.x-1, p.y+1));
        if (p.x < sizeX-1 && p.y > 0) f(point(p.x+1, p.y-1));
        if (p.x < sizeX-1 && p.y < sizeY-1) f(point(p.x+1, p.y+1));
    }

    template<typename F>
    void forRectangle(const Pos &p, const Pos &s, F f) {
        assert(isPoint(p));
        assert(isPoint(p + s));
        for (size_t x = p.x; x <= p.x + s.x; x++)
            for (size_t y = p.y; y <= p.y + s.y; y++)
                f(point(x, y));
    }
    template<typename F>
    void forRectangle(const Pos &p, const Pos &s, F f) const {
        assert(isPoint(p));
        assert(isPoint(p + s));
        for (size_t x = p.x; x <= p.x + s.x; x++)
            for (size_t y = p.y; y <= p.y + s.y; y++)
                f(point(x, y));
    }

    void raise(const Pos &p, const Pos &s);
    void tick(Fixed tickLengthS);

private:
    size_t sizeX;
    size_t sizeY;

    size_t maxHeight;

    std::vector<GridPoint> points; // 2d array
};

#endif
