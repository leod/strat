#ifndef STRAT_GAME_SIM_SYSTEMS_HH
#define STRAT_GAME_SIM_SYSTEMS_HH

#include <entityx/entityx.h>

#include "SimComponents.hh"

struct SimState;

struct MinerBuildingSystem {
    void tick(SimState &);    
};

struct FlyingObjectSystem {
    void tick(SimState &);    
};

struct FlyingResourceSystem : entityx::Receiver<FlyingResourceSystem> {
    FlyingResourceSystem(SimState &simState)
        : simState(simState) {
    }

    void configure(entityx::EventManager &);
    void receive(const FlyingObjectLanded &);

    SimState &simState;
};

struct FlyingBlockSystem : entityx::Receiver<FlyingBlockSystem> {
    FlyingBlockSystem(entityx::EntityManager &entities)
        : entities(entities) {
    }

    void configure(entityx::EventManager &);
    void receive(const FlyingObjectLanded &);

    entityx::EntityManager &entities;
};

struct RocketSystem : entityx::Receiver<RocketSystem> {
    RocketSystem(Map &map)
        : map(map) {
    }

    void configure(entityx::EventManager &);
    void receive(const FlyingObjectLanded &);

private:
    Map &map;
};

struct MainBuildingSystem {
    void tick(SimState &);
};

#endif
