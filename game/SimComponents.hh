#ifndef STRAT_GAME_SIM_COMPONENTS_HH
#define STRAT_GAME_SIM_COMPONENTS_HH

#include "Map.hh"
#include "Math.hh"
#include "common/Defs.hh"

#include <entityx/entityx.h>
#include <Fixed.hh>

using entityx::Entity;

struct GameObject : entityx::Component<GameObject> {
    GameObject(PlayerId owner, ObjectId id)
        : owner(owner),
          id(id) {
    }

    PlayerId getOwner() const { return owner; }
    ObjectId getId() const { return id; }

private:
    PlayerId owner;
    ObjectId id;
};

struct Building : entityx::Component<Building> {
    Building(BuildingType type, const glm::uvec3 &position)
        : type(type), position(position) {
        assert(type >= 0 && type < BUILDING_MAX);
    }

    BuildingType getType() const { return type; } 
    BuildingTypeInfo &getTypeInfo() const {
        return buildingTypeInfo[type];
    }

    const glm::uvec3 &getPosition() const { return position; }

private:
    const BuildingType type;

    glm::uvec3 position;
};

struct MinerBuilding : entityx::Component<MinerBuilding> {
    friend struct MinerBuildingSystem;

    MinerBuilding(ResourceType resource)
        : resource(resource), amountStored(0) {
    }

private:
    const ResourceType resource;

    Fixed amountStored;
};

struct Tree : entityx::Component<Tree> {
    Tree(const glm::uvec3 &position)
        : position(position) {
    }   

    const glm::uvec3 &getPosition() const { return position; }

private:
    glm::uvec3 position;
};

struct FlyingObjectLanded : public entityx::Event<FlyingObjectLanded> {
    entityx::Entity entity; 

    FlyingObjectLanded(entityx::Entity entity)
        : entity(entity) {
    }
};

struct FlyingObject : public entityx::Component<FlyingObject> {
private:
    Fixed lastProgress; // e.g. for interpolation
    Fixed progress;

public:
    friend struct FlyingObjectSystem;

    const fvec3 fromPosition;
    const fvec3 toPosition;
    const Fixed distance;

    FlyingObject(fvec3 fromPosition, fvec3 toPosition)
        : lastProgress(0), progress(0),
          fromPosition(fromPosition), toPosition(toPosition),
          distance(manhattanDistance(fromPosition, toPosition)) {
        assert(distance > Fixed(0));
    }

    Fixed getProgress() const { return progress; }
    Fixed getLastProgress() const { return lastProgress; }
};

struct FlyingResource : public entityx::Component<FlyingResource> {
    const ResourceType resource;
    const size_t amount;

    const glm::vec3 color;

    FlyingResource(ResourceType resource, size_t amount)
        : resource(resource),
          amount(amount),
          color(randomFloat(), randomFloat(), randomFloat()) {
    }
};

struct BuildingCreated : public entityx::Event<BuildingCreated> {
    entityx::Entity entity;

    BuildingCreated(entityx::Entity entity)
        : entity(entity) {
    }
};

#endif
