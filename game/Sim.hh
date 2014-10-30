#ifndef STRAT_GAME_SIM_HH
#define STRAT_GAME_SIM_HH

#include "Map.hh"
#include "SimComponents.hh"
#include "SimSystems.hh"
#include "SimState.hh"
#include "common/GameSettings.hh"
#include "common/Order.hh"

#include <entityx/entityx.h>

#include <vector>
#include <cassert>

struct Sim {
    Sim(const GameSettings &);

    void runTick(const std::vector<Order> &orders);

    const SimState &getState() const;

    entityx::EntityManager &getEntities() {
        return state.entities;
    }

    const entityx::EntityManager &getEntities() const {
        return state.entities;
    }

private:
    SimState state;

    MinerBuildingSystem minerBuildingSystem;
    ResourceTransferSystem resourceTransferSystem;
};

#endif
