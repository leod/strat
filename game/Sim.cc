#include "game/Sim.hh"

#include "Map.hh"

Sim::Sim(const GameSettings &settings)
    : state(settings) {
}

void Sim::runTick(const std::vector<Order> &orders) {
    for (auto &order : orders) {
        if (state.isOrderValid(order))
            state.runOrder(order);
    }

    minerBuildingSystem.tick(state);
    resourceTransferSystem.tick(state);
}

const SimState &Sim::getState() const {
    return state;
}
