#ifndef STRAT_GAME_GRAPHICS_HH
#define STRAT_GAME_GRAPHICS_HH

#include "Config.hh"
#include "Math.hh"
#include "Sim.hh"
#include "Map.hh"

#include <entityx/entityx.h>
#include <GL/glew.h>

#include "Input.hh"

struct Map;
struct InterpState;

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
