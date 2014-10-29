#ifndef STRAT_GAME_SIM_STATE_HH
#define STRAT_GAME_SIM_STATE_HH

#include "Map.hh"
#include "common/GameSettings.hh"
#include "common/Order.hh"

#include <entityx/entityx.h>

// Contains all the relevant information about the game state,
// so that the next state can be calculated deterministically.
struct SimState : entityx::EntityX {
    SimState(const GameSettings &);

    bool canPlaceBuilding(BuildingType, size_t px, size_t py) const;
    bool isOrderValid(const Order &) const;

    void runOrder(const Order &);
    
    Map &getMap() { return map; }
    const Map &getMap() const { return map; }

private:
    GameSettings settings;
    Map map;

    size_t entityCounter;
};

#endif
