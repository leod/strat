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

// Sim holds all the game logic.
//
// The state of the game is stored in a SimState.
// Note that from the outside you can not change the SimState!
//
// The game is divided into discrete time units called ticks.
// Usually, the clients will receive for every tick a list of
// orders (made by the players) to be performed that tick.
// The only part where the SimState is meant to be changed
// is in the function executing those orders: Sim::runTick.
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

    entityx::EventManager &getEvents() {
        return state.events;
    }

private:
    SimState state;

    MinerBuildingSystem minerBuildingSystem;
    FlyingObjectSystem flyingObjectSystem;
    RocketSystem rocketSystem;
};

#endif
