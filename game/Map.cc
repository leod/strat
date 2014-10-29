#include "Map.hh"

#include "Math.hh"

Map::Map(size_t sizeX, size_t sizeY)
    : sizeX(sizeX)
    , sizeY(sizeY)
    , maxHeight(0)
    , points(sizeX * sizeY) {
} 

Map *Map::generate(size_t sizeX, size_t sizeY, int heightLimit) {
    Map *map = new Map(sizeX, sizeY);

    PerlinNoise noise(sizeX, sizeY);
    noise.generate(8, 0.4);

    int minHeight = heightLimit;

    // Create heightmap from perlin noise,
    // converting from floats to integer values
    for (size_t x = 0; x < sizeX; x++) {
        for (size_t y = 0; y < sizeY; y++) {
            float r = noise.get(x, y);

            map->point(x, y).height =
                static_cast<int>(lerp<float>(0, heightLimit, r));
            if (map->point(x, y).height < minHeight)
                minHeight = map->point(x, y).height;
        }
    }

    // Floor map, so that lowest points have height zero
    for (size_t x = 0; x < sizeX; x++) {
        for (size_t y = 0; y < sizeY; y++) {
            map->point(x, y).height -= minHeight;

            if (map->point(x, y).height > map->maxHeight)
                map->maxHeight = map->point(x, y).height;
        }
    }

    return map; 
}
