#ifndef STRAT_GAME_SIM_COMPONENTS_HH
#define STRAT_GAME_SIM_COMPONENTS_HH

#include "Map.hh"
#include "common/Defs.hh"

#include <entityx/entityx.h>

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

struct MinerBuilding : public entityx::Component<MinerBuilding> {
    friend struct MinerSystem;

    MinerBuilding(ResourceType resource)
        : resource(resource), amountStored(0) {
    }

private:
    const ResourceType resource;

    size_t amountStored;
};

struct ResourceTransfer : public entityx::Component<ResourceTransfer> {
    friend struct ResourceTransferSystem;

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

#endif
