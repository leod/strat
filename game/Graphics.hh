#ifndef STRAT_GAME_GRAPHICS_HH
#define STRAT_GAME_GRAPHICS_HH

#include "Config.hh"
#include "Math.hh"
#include "Sim.hh"
#include "Map.hh"

#include <entityx/entityx.h>
#include <GL/glew.h>

struct Map;
struct InterpState;

struct View {
    float targetX;
    float targetY;

    float distance;

    float height;
};

struct Vertex {
    glm::vec3 position, color, normal;

    Vertex(glm::vec3 position,
           glm::vec3 color,
           glm::vec3 normal)
        : position(position)
        , color(color)
        , normal(normal) {
    }
};

struct TerrainMesh {
    TerrainMesh(const Map &);

    void draw();

private:
    void init();

    glm::vec3 color(size_t height);

    const Map &map;

    std::vector<Vertex> vertices;
    GLuint vertexBuffer;
};

struct RenderBuilding : public entityx::Component<RenderBuilding> {
};

struct RenderBuildingSystem :
    public entityx::Receiver<RenderBuildingSystem> {
    RenderBuildingSystem(const Map &map) : map(map) {}

    void configure(entityx::EventManager &);
    void receive(const BuildingCreated &);

    void render(entityx::EntityManager &entities);

private:
    const Map &map;
};

struct RenderResourceTransferSystem {
    RenderResourceTransferSystem(const Map &map, const InterpState &interp)
        : map(map), interp(interp) {}

    void render(entityx::EntityManager &entities);

private:
    const Map &map;
    const InterpState &interp;
};

void setupGraphics(const Config &, const View &);
void drawCursor(const View &view);

#endif
