#ifndef STRAT_GAME_SIM_STATE_HH
#define STRAT_GAME_SIM_STATE_HH

#include "Map.hh"
#include "common/GameSettings.hh"
#include "common/Order.hh"

#include <entityx/entityx.h>
#include <Fixed.hh>
#include <map>

using entityx::Entity;

struct PlayerState {
    PlayerState(const PlayerInfo &info);

    void giveResources(ResourceType type, size_t amount) {
        resources.at(type) += amount;
    }

    const std::vector<size_t> &getResources() const {
        return resources;
    }

private:
    const PlayerInfo &info;

    std::vector<size_t> resources;
};

// Contains all the relevant information about the game state,
// so that the next state can be calculated deterministically.
struct SimState : entityx::EntityX {
    SimState(const GameSettings &);

    bool canPlaceBuilding(BuildingType, const glm::uvec2 &p) const;
    entityx::Entity findClosestBuilding(BuildingType, PlayerId,
                                        const glm::uvec2 &p,
                                        size_t maxRange) const;

    bool isOrderValid(const Order &) const;
    void runOrder(const Order &);
    
    Map &getMap() { return map; }
    const Map &getMap() const { return map; }

    PlayerState &getPlayer(PlayerId);
    const PlayerState &getPlayer(PlayerId) const;

    // Tick length in seconds
    Fixed getTickLengthS() const;

    void addResourceTransfer(Entity fromPosition, Entity toPosition,
                             ResourceType resource,
                             size_t amount); 

private:
    const GameSettings &settings;
    Map map;
    
    typedef std::map<PlayerId, PlayerState> PlayerMap;
    PlayerMap players;

    size_t entityCounter;

    static PlayerMap playersFromSettings(const GameSettings &);
};

#endif
