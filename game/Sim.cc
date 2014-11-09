#include "Sim.hh"

#include "Map.hh"
#include "util/Profiling.hh"

Sim::Sim(const GameSettings &settings)
    : state(settings),
      rocketSystem(state.getMap()) {
    state.raiseWaterLevel();
    state.raiseWaterLevel();

    rocketSystem.configure(state.events);
}

void Sim::runTick(const std::vector<Order> &orders) {
    PROFILE(tick);

    for (auto &order : orders) {
        if (state.isOrderValid(order))
            state.runOrder(order);
    }

    state.getMap().tick(state.entities, state.getTickLengthS());
    state.waterTick();

    minerBuildingSystem.tick(state);
    flyingObjectSystem.tick(state);
    //rocketSystem.tick(state);
}

const SimState &Sim::getState() const {
    return state;
}
