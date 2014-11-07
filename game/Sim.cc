#include "game/Sim.hh"

#include "Map.hh"

Sim::Sim(const GameSettings &settings)
    : state(settings) {
    state.raiseWaterLevel();
    state.raiseWaterLevel();
}

void Sim::runTick(const std::vector<Order> &orders) {
    for (auto &order : orders) {
        if (state.isOrderValid(order))
            state.runOrder(order);
    }

    state.getMap().tick(state.getTickLengthS());
    state.waterTick();

    minerBuildingSystem.tick(state);
    flyingObjectSystem.tick(state);
}

const SimState &Sim::getState() const {
    return state;
}
