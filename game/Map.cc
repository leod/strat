#include "Map.hh"

#include "Math.hh"

#include <cstdlib>

Map::Map(size_t sizeX, size_t sizeY)
    : sizeX(sizeX),
      sizeY(sizeY),
      maxHeight(0),
      points(sizeX * sizeY) {
    assert(sizeX > 0 && sizeY > 0);
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
    });
}

void Map::tick(Fixed tickLengthS) {
    Fixed growthPerS = Fixed(1);

    for (size_t x = 0; x < sizeX; x++) {
        for (size_t y = 0; y < sizeY; y++) {
            GridPoint &p(point(x, y));

            if (p.growthTarget > 0 && p.growthProgress < Fixed(1)) {
                p.growthProgress += growthPerS * tickLengthS;

                if (p.growthProgress >= Fixed(1)) {
                    p.height += 1;
                    p.growthTarget--;
                    p.growthProgress -= Fixed(1);
                    p.growthCascade = true;
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

            if (p.growthCascade) {
                forNeighbors(Map::Pos(x, y), [&] (GridPoint &n) {
                    if (n.height + n.growthTarget + 1 < p.height)
                        n.growthTarget += p.height - (n.height + n.growthTarget + 1);
                });

                p.growthCascade = false;
            }
        }
    }
}
