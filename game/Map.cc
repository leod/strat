#include "Map.hh"

#include "Math.hh"

#include <cstdlib>

Map::Map(size_t sizeX, size_t sizeY)
    : sizeX(sizeX),
      sizeY(sizeY),
      maxHeight(0),
      points(sizeX * sizeY) {
    assert(sizeX > 0 && sizeY > 0);

    for (size_t x = 0; x < sizeX; x++) {
        for (size_t y = 0; y < sizeY; y++) {
            point(x, y).pos = Map::Pos(x, y);
        }
    } 
} 

Map Map::generate(size_t sizeX, size_t sizeY,
                  size_t heightLimit, size_t seed) {
    // HACK: Maybe use C++11's random generators to ensure determinism
    srand(seed);

    Map map(sizeX, sizeY);

    PerlinNoise noise(sizeX, sizeY);
    noise.generate(8, 0.4);

    size_t minHeight = heightLimit;

    // Create heightmap from perlin noise,
    // converting from floats to integer values
    for (size_t x = 0; x < sizeX; x++) {
        for (size_t y = 0; y < sizeY; y++) {
            float r = noise.get(x, y);

            map.point(x, y).height =
                static_cast<int>(lerp<float>(0, heightLimit, r));
            if (map.point(x, y).height < minHeight)
                minHeight = map.point(x, y).height;
        }
    }

    // Floor map, so that lowest points have height zero
    for (size_t x = 0; x < sizeX; x++) {
        for (size_t y = 0; y < sizeY; y++) {
            map.point(x, y).height -= minHeight;

            if (map.point(x, y).height > map.maxHeight)
                map.maxHeight = map.point(x, y).height;
        }
    }

    return std::move(map); 
}

void Map::crater(const Pos &p, size_t depth) {
    GridPoint &gp(point(p));
    gp.growthTarget = -static_cast<int>(std::min(gp.height, depth));
    gp.growthPerS = Fixed(8);

    growingPoints.insert(p);
}

void Map::raise(const Pos &p, const Pos &s) {
    size_t maxRectHeight = 0;
    bool allMaxHeight = true;
    forRectangle(p, s, [&] (GridPoint &p) {
        if (p.height > maxRectHeight)
            maxRectHeight = p.height;

        allMaxHeight = allMaxHeight && p.height == maxRectHeight;
    });

    if (allMaxHeight)
        maxRectHeight++;

    forRectangle(p, s, [&] (GridPoint &p) {
        p.growthTarget = maxRectHeight - p.height;
        p.growthPerS = Fixed(1);

        growingPoints.insert(p.pos);
    });
}

void Map::tick(entityx::EntityManager &entities, Fixed tickLengthS) {
    // Grow singular points
    for (auto it = growingPoints.begin(); it != growingPoints.end(); ) {
        GridPoint &p(point(*it));
        p.dirty = false;

        // Don't grow below 0
        assert(static_cast<int>(p.height) + p.growthTarget >= 0);

        // Point currently growing?
        if (p.growthTarget != 0 && p.growthProgress < Fixed(1)) {
            if (p.entity) { 
                // Growing destroys entities on that point
                entities.destroy(p.entity.id());
            }

            p.growthProgress += p.growthPerS * tickLengthS;
            p.dirty = true;

            if (p.growthProgress >= Fixed(1)) {
                if (p.growthTarget > 0) {
                    // Grow upward
                    p.height += 1;
                    p.growthTarget--;
                    p.growthCascadeUp = true;
                } else if (p.height > 0) {
                    // Grow downward
                    p.height--;
                    p.growthTarget++;
                    p.growthCascadeDown = true;
                }

                p.growthProgress -= Fixed(1);
                p.water = Fixed(0);

                if (p.growthTarget == 0) // Stop growing
                    p.growthProgress = Fixed(0);
            }
        } else {
            // If not growing, remove from set 
            growingPoints.erase(it++);
            continue;
        }
        
        ++it;
    }

    // Cascade growth from points that have finished growing
    for (auto it = growingPoints.begin(); it != growingPoints.end(); ++it) {
        GridPoint &p(point(*it));

        if (p.growthCascadeUp) {
            forNeighbors(*it, [&] (GridPoint &n) {
                // For every neighbor that is more than 1 height lower than us,
                if (n.height + n.growthTarget + 1 < p.height) {
                    // raise their height, so that the height difference is at most 1
                    n.growthTarget += p.height - (n.height + n.growthTarget + 1);
                    n.growthPerS = p.growthPerS;

                    growingPoints.insert(n.pos);
                }
            });

            p.growthCascadeUp = false;
        }
        if (p.growthCascadeDown) {
            forNeighbors(*it, [&] (GridPoint &n) {
                // For every neighbor that is more than 1 height higher than us,
                if (n.height + n.growthTarget > p.height + 1) {
                    // lower their height, so that the height difference is at most 1
                    n.growthTarget += static_cast<int>(p.height + 1) -
                                      static_cast<int>(n.height + n.growthTarget);
                    n.growthPerS = p.growthPerS;

                    growingPoints.insert(n.pos);
                }
            });

            p.growthCascadeDown = false;
        }
    }
}

void Map::raiseWaterLevel(size_t waterLevel) {
    for (size_t x = 0; x < getSizeX(); x++) {
        for (size_t y = 0; y < getSizeY(); y++) {
            GridPoint &p(point(x, y));

            if (p.height == 0 && waterLevel > 0)
                p.waterSource = true;
        }
    }
}

void Map::waterTick(Fixed tickLengthS, size_t waterLevel) {
    Fixed flowPerS(Fixed(4) / Fixed(1));

    // Calculate water flow from grid point to grid point
    for (size_t x = 0; x < map.getSizeX(); x++) {
        for (size_t y = 0; y < map.getSizeY(); y++) {
            GridPoint &p(map.point(x, y));

            if (p.height < waterLevel) {
                if (p.water < Fixed(waterLevel - p.height))*/) {
                    map.forNeighbors(Map::Pos(x, y), [&] (GridPoint &p2) {
                        if (p2.water == Fixed(0)) return;
                        if (Fixed(p2.height) + p2.water < Fixed(p.height) + p.water) return;

                        Fixed value((Fixed(p2.height) + p2.water) - (Fixed(p.height) + p.water));

                        value *= flowPerS * getTickLengthS();

                        p2.water -= value;
                        p.water += value;
                    });
                }
                
                if (p.water < Fixed(waterLevel - p.height) && p.waterSource) {
                    // This point has been flooded for a longer time,
                    // but hasn't risen to the waterLevel yet

                    p.water += Fixed(1) * getTickLengthS();
                }

                // Don't allow flooding above the global water level
                if (p.water > Fixed(waterLevel - p.height))
                    p.water = Fixed(waterLevel - p.height);
            }
        }
    }
}

