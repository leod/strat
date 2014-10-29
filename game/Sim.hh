#ifndef STRAT_GAME_SIM_HH
#define STRAT_GAME_SIM_HH

#include "Map.hh"
#include "common/Order.hh"
#include "common/GameSettings.hh"

#include <entityx/entityx.h>

#include <vector>
#include <cassert>

struct GameObject : public entityx::Component<GameObject> {
    GameObject(PlayerId owner, ObjectId id)
        : owner(owner)
        , id(id) {
    }

    PlayerId getOwner() const { return owner; }
    ObjectId getId() const { return id; }

private:
    const PlayerId owner;
    const ObjectId id;
};

struct Building : public entityx::Component<Building> {
    Building(BuildingType type, size_t x, size_t y)
        : type(type), x(x), y(y) {
        assert(type >= 0 && type < BUILDING_MAX);
    }

    BuildingType getType() const { return type; } 
    BuildingTypeInfo &getTypeInfo() const {
        return buildingTypeInfo[type];
    }

    size_t getX() const { return x; }
    size_t getY() const { return y; }

private:
    const BuildingType type;

    size_t x, y;
};

struct ResourceTransferSystem {

};

struct ResourceTransfer : public entityx::Component<ResourceTransfer> {
    friend ResourceTransferSystem;

private:
    size_t numSteps;
    size_t curStep; 
};

struct BuildingCreated : entityx::Event<BuildingCreated> {
    BuildingCreated(entityx::Entity entity)
        : entity(entity) {
    }

    entityx::Entity entity;
};

// Contains all the relevant information about the game state,
// so that the next state can be calculated deterministically.
struct Sim {
    Sim(entityx::EntityX &entityx);

    void start(const GameSettings &settings);

    bool canPlaceBuilding(BuildingType, size_t px, size_t py) const;
    bool isOrderValid(const Order &) const;

    void runOrder(const Order &);
    void runTick(const std::vector<Order> &orders);

    Map &getMap() { assert(map); return *map; }
    const Map &getMap() const { assert(map); return *map; }
 
    bool isStarted() const { return started; }

private:
    entityx::EntityX &entityx; 

    bool started;

    Map *map;
    size_t entityCounter;

    GameSettings settings;
};

#endif
