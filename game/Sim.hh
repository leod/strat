#ifndef STRAT_GAME_SIM_HH
#define STRAT_GAME_SIM_HH

#include "Map.hh"
#include "SimComponents.hh"
#include "SimSystems.hh"
#include "SimState.hh"
#include "common/GameSettings.hh"
#include "common/Order.hh"

#include <vector>
#include <cassert>

struct Sim {
    Sim(const GameSettings &);

    void runTick(const std::vector<Order> &orders);

    SimState &getState();
    const SimState &getState() const;

private:
    SimState state;
};

#endif
