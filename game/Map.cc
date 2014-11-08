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
    });
}

void Map::tick(entityx::EntityManager &entities, Fixed tickLengthS) {
    for (size_t x = 0; x < sizeX; x++) {
        for (size_t y = 0; y < sizeY; y++) {
            GridPoint &p(point(x, y));

            assert(static_cast<int>(p.height) + p.growthTarget >= 0);

            if (p.growthTarget != 0 && p.growthProgress < Fixed(1)) {
                if (p.entity) { 
                    entities.destroy(p.entity.id());
                }

                p.growthProgress += p.growthPerS * tickLengthS;

                if (p.growthProgress >= Fixed(1)) {
                    if (p.growthTarget > 0) {
                        p.height += 1;
                        p.growthTarget--;
                        p.growthCascadeUp = true;
                    } else if (p.height > 0) {
                        p.height--;
                        p.growthTarget++;
                        p.growthCascadeDown = true;
                    }

                    p.growthProgress -= Fixed(1);
                    p.water = Fixed(0);

                    if (p.growthTarget == 0) // Stop growing
                        p.growthProgress = Fixed(0);
                }
            }
        }
    }

    for (size_t x = 0; x < sizeX; x++) {
        for (size_t y = 0; y < sizeY; y++) {
            GridPoint &p(point(x, y));

            if (p.growthCascadeUp) {
                forNeighbors(Map::Pos(x, y), [&] (GridPoint &n) {
                    // For every neighbor that is more than 1 height lower than us,
                    if (n.height + n.growthTarget + 1 < p.height) {
                        // raise their height, so that the height difference is at most 1
                        n.growthTarget += p.height - (n.height + n.growthTarget + 1);
                        n.growthPerS = p.growthPerS;
                    }
                });

                p.growthCascadeUp = false;
            }
            if (p.growthCascadeDown) {
                forNeighbors(Map::Pos(x, y), [&] (GridPoint &n) {
                    // For every neighbor that is more than 1 height higher than us,
                    if (n.height + n.growthTarget > p.height + 1) {
                        // lower their height, so that the height difference is at most 1
                        n.growthTarget += static_cast<int>(p.height + 1) -
                                          static_cast<int>(n.height + n.growthTarget);
                        n.growthPerS = p.growthPerS;
                    }
                });

                p.growthCascadeDown = false;
            }
        }
    }
}
