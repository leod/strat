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

    void giveResources(ResourceType type, size_t amount = 1) {
        resources.at(type) += amount;
    }

    const Resources &getResources() const {
        return resources;
    }

    bool haveResources(const Resources &target) const {
        for (size_t i = 0; i < resources.size(); i++) {
            if (resources[i] >= target[i])
                return true;
        }

        return false;
    }

    bool haveResources(ResourceType type, size_t amount = 1) {
        return resources[type] >= amount;
    }

    void takeResources(const Resources &target) {
        assert(haveResources(target));

        for (size_t i = 0; i < resources.size(); i++) {
            resources[i] -= target[i];
        }
    }

    void takeResources(ResourceType type, size_t amount = 1) {
        assert(haveResources(type, amount));

        resources[type] -= amount;
    }

private:
    const PlayerInfo &info;

    Resources resources;
};

// Contains all the relevant information about the game state,
// so that the next state can be calculated deterministically.
//
// This state includes:
// - the map, which stores the heights of the grid points and the water state,
// - information about players, and
// - the game entities which are managed as components using EntityX
//
// SimState also offers several functions to modify the game state.
struct SimState : entityx::EntityX {
    SimState(const GameSettings &);

    bool canPlaceBuilding(BuildingType, const glm::uvec2 &p) const;
    entityx::Entity findClosestBuilding(BuildingType, PlayerId,
                                        const glm::uvec2 &p,
                                        size_t maxRange) const;

    void placeTree(const glm::uvec2 &p);

    bool isOrderValid(const Order &) const;
    void runOrder(const Order &);
    
    Map &getMap() { return map; }
    const Map &getMap() const { return map; }

    PlayerState &getPlayer(PlayerId);
    const PlayerState &getPlayer(PlayerId) const;

    entityx::Entity getGameObject(ObjectId) const;

    // Tick length in seconds
    Fixed getTickLengthS() const;

    // Time elapsed in simulation in seconds
    Fixed getTimeS() const { return time; }

    entityx::Entity addBuilding(PlayerId owner, BuildingType type,
                                const glm::uvec2 &position, bool finished);
    entityx::Entity addFlyingResource(Entity fromEntity, Entity toEntity,
                                      ResourceType resource,
                                      size_t amount); 
    entityx::Entity addFlyingBlock(Entity fromEntity, Entity toEntity,
                                   const BuildingTypeInfo::Block &block);
    entityx::Entity addRocket(Entity fromEntity, Map::Pos toPos);

    void raiseWaterLevel();
    size_t getWaterLevel() const { return waterLevel; }

    void tick();

private:
    const GameSettings &settings;
    Map map;
    
    typedef std::map<PlayerId, PlayerState> PlayerMap;
    PlayerMap players;

    size_t entityCounter;

    Fixed time;

    size_t waterLevel;

    static PlayerMap playersFromSettings(const GameSettings &);
};

#endif
