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
    forRectangle(p, s, [&] (GridPoint &p) {
        //p.height++; 
        p.isGrowing = true;
    });
}

void Map::tick(Fixed tickLengthS) {
    Fixed growthPerS = Fixed(1);

    for (size_t x = 0; x < sizeX; x++) {
        for (size_t y = 0; y < sizeY; y++) {
            GridPoint &p(point(x, y));

            if (p.isGrowing) {
                p.growthProgress += growthPerS * tickLengthS;

                if (p.growthProgress >= Fixed(1)) {
                    p.height++;
                    p.isGrowing = false;
                    p.growthProgress = 0;
                }
            }
        }
    }
}
