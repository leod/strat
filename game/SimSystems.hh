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
    void configure(entityx::EventManager &);
    void receive(const FlyingObjectLanded &);
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

#endif
