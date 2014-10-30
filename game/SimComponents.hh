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

    const Fixed fromX;
    const Fixed fromY;
    const Fixed fromZ;
    const Fixed toX;
    const Fixed toY;
    const Fixed toZ;
    const ResourceType resource;
    const size_t amount;
    const Fixed distance;

    ResourceTransfer(Entity fromEntity, Entity toEntity,
                     Fixed fromX, Fixed fromY, Fixed fromZ,
                     Fixed toX, Fixed toY, Fixed toZ,
                     ResourceType resource,
                     size_t amount)
        : lastProgress(0), progress(0),
          fromEntity(fromEntity),
          toEntity(toEntity),
          fromX(fromX), fromY(fromY), fromZ(fromZ),
          toX(toX), toY(toY), toZ(toZ),
          resource(resource),
          amount(amount),
          distance(((std::max(fromX, toX) - std::min(fromX, toX)) +
                   ((std::max(fromY, toY) - std::min(fromY, toY))))) {
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
