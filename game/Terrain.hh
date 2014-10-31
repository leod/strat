#ifndef STRAT_GAME_TERRAIN_HH
#define STRAT_GAME_TERRAIN_HH

#include "Math.hh"
#include "Map.hh"

#include <GL/glew.h>

struct TerrainPatch;

struct TerrainMesh {
    TerrainMesh(const Map &);

    void draw();

private:
    void init();

    glm::vec3 color(size_t height);

    const Map &map;
    std::vector<TerrainPatch *> patches;
};

#endif
