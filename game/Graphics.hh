#ifndef STRAT_GAME_GRAPHICS_HH
#define STRAT_GAME_GRAPHICS_HH

#include "Config.hh"
#include "Math.hh"
#include "Sim.hh"
#include "Map.hh"
#include "Input.hh"

#include <entityx/entityx.h>
#include <GL/glew.h>

#include "opengl/OBJ.hh"
#include "opengl/TextureManager.hh"

struct Map;
struct InterpState;

struct RenderBuildingSystem {
    RenderBuildingSystem(const Map &map, const Input &input)
        : map(map), input(input) {}

    void render(entityx::EntityManager &entities);

    void renderBuilding(BuildingType, const glm::uvec3 &,
                        const std::vector<BuildingTypeInfo::Block> &);

private:
    const Map &map;
    const Input &input;
};

struct RenderFlyingResourceSystem {
    RenderFlyingResourceSystem(const Map &map, const InterpState &interp)
        : map(map), interp(interp) {
    }

    void render(entityx::EntityManager &entities);

private:
    const Map &map;
    const InterpState &interp;
};

struct RenderRocketSystem {
    RenderRocketSystem(const InterpState &interp)
        : interp(interp) {
    }

    void render(entityx::EntityManager &entities);

private:
    const InterpState &interp;
};

struct RenderTreeSystem {
    RenderTreeSystem(const Map &map, opengl::TextureManager &textures)
        : map(map), treeObj("data/tree/untitled.obj", textures) {}

    void render(entityx::EntityManager &entities);

private:
    const Map &map;

    opengl::OBJ treeObj;
};

void setupGraphics(const Config &, const Input::View &);
void drawCursor(const Map &, const Input &);

#endif
