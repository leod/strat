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
    const PlayerId owner;
    const ObjectId id;
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

class ResourceTransfer : public entityx::Component<ResourceTransfer> {
    Fixed lastProgress; // e.g. for interpolation
    Fixed progress;

public:
    friend struct ResourceTransferSystem;

    const Entity fromEntity;
    const Entity toEntity;

    const fvec3 fromPosition;
    const fvec3 toPosition;
    const ResourceType resource;
    const size_t amount;
    const Fixed distance;

    ResourceTransfer(Entity fromEntity, Entity toEntity,
                     fvec3 fromPosition, fvec3 toPosition,
                     ResourceType resource,
                     size_t amount)
        : lastProgress(0), progress(0),
          fromEntity(fromEntity),
          toEntity(toEntity),
          fromPosition(fromPosition), toPosition(toPosition),
          resource(resource),
          amount(amount),
          distance(manhattanDistance(fromPosition, toPosition)) {
        assert(distance > Fixed(0));
    }

    Fixed getProgress() const { return progress; }
    Fixed getLastProgress() const { return lastProgress; }
};

struct BuildingCreated : public entityx::Event<BuildingCreated> {
    BuildingCreated(entityx::Entity entity)
        : entity(entity) {
    }

    entityx::Entity entity;
};

#endif
