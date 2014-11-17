#ifndef STRAT_GAME_SIM_COMPONENTS_HH
#define STRAT_GAME_SIM_COMPONENTS_HH

#include "Map.hh"
#include "Math.hh"
#include "common/Defs.hh"

#include <entityx/entityx.h>
#include <Fixed.hh>

#include <algorithm>

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

struct FlyingBlock : entityx::Component<FlyingBlock> {
    entityx::Entity targetEntity;
    const BuildingTypeInfo::Block block;

    FlyingBlock(entityx::Entity targetEntity,
                const BuildingTypeInfo::Block &block)
        : targetEntity(targetEntity), block(block) {
    }
};

struct Building : entityx::Component<Building> {
    friend struct MainBuildingSystem;

    Building(BuildingType type, const glm::uvec3 &position,
             const GridPoint &gridPoint, bool finished)
        : type(type), position(position), gridPoint(gridPoint), finished(finished) {
        assert(type >= 0 && type < BUILDING_MAX);

        if (finished)
            blocks = getTypeInfo().blocks;
    }

    BuildingType getType() const { return type; } 
    BuildingTypeInfo &getTypeInfo() const {
        return buildingTypeInfo[type];
    }

    const glm::uvec3 &getPosition() const { return position; }

    const std::vector<BuildingTypeInfo::Block> &getBlocks() const { 
        return blocks;
    }

    void addBlock(const BuildingTypeInfo::Block &block) {
        if (finished)
            return;

        blocks.push_back(block); 

        // Building finished?
        finished = true;
        for (const auto &block : getTypeInfo().blocks) {
            if (std::find(blocks.begin(), blocks.end(), block) == blocks.end())
                finished = false;
        }
    }

    void addIncomingBlock(FlyingBlock::Handle flyingBlock) {
        incomingBlocks.push_back(flyingBlock);
    }
    
    bool isBlockNeeded(const BuildingTypeInfo::Block &block) const {
        if (std::find(blocks.begin(), blocks.end(), block) != blocks.end())
            return false;

        for (auto incoming : incomingBlocks) {
            if (incoming && incoming->block == block)
                return false;
        }

        return std::find(getTypeInfo().blocks.begin(),
                         getTypeInfo().blocks.end(),
                         block) != getTypeInfo().blocks.end();
    }

    bool isBlockNeeded() const {
        // Might need to make these parts more efficient if many blocks are used

        for (const auto &block : getTypeInfo().blocks) {
            if (isBlockNeeded(block))
                return true;
        }

        return false;
    }

    BuildingTypeInfo::Block nextNeededBlock() const {
        assert(!finished);

        for (const auto &block : getTypeInfo().blocks) {
            if (isBlockNeeded(block))
                return block;
        }

        assert(false);
    }

    bool isFinished() const { return finished; }

    bool isUsable() const {
        return isFinished()
               && gridPoint.water == Fixed(0);
    }

private:
    const BuildingType type;
    glm::uvec3 position;
    const GridPoint &gridPoint;
    bool finished; // finished building

    // Resources the building needs in order to be built
    std::vector<BuildingTypeInfo::Block> blocks;

    std::vector<FlyingBlock::Handle> incomingBlocks;
};

struct MainBuilding : entityx::Component<MainBuilding> {
    friend struct MainBuildingSystem;

    size_t getBuildRange() const { return 100; }
    Fixed getBuildSpeed() const { return Fixed(1); }

    void build(entityx::Entity entity) {
        assert(entity.has_component<Building>());

        buildQueue.clear();
        buildQueue.push_back(entity);
    }

    void queueBuild(entityx::Entity entity) {
        assert(entity.has_component<Building>());

        buildQueue.push_back(entity);
    }

    void removeFromQueue(entityx::Entity entity) {
        auto it = std::find(buildQueue.begin(),
                            buildQueue.end(),
                            entity);
        assert(it != buildQueue.end());
        buildQueue.erase(it);
    }

private:
    std::vector<entityx::Entity> buildQueue;

    Fixed timeLastLaunch;
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

struct FlyingObjectLanded : entityx::Event<FlyingObjectLanded> {
    entityx::Entity entity; 

    FlyingObjectLanded(entityx::Entity entity)
        : entity(entity) {
    }
};

struct FlyingObject : entityx::Component<FlyingObject> {
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

struct FlyingResource : entityx::Component<FlyingResource> {
    const ResourceType resource;
    const size_t amount;

    const glm::vec3 color;

    FlyingResource(ResourceType resource, size_t amount)
        : resource(resource),
          amount(amount),
          color(randomFloat(), randomFloat(), randomFloat()) {
    }
};

struct Rocket : entityx::Component<Rocket> {

};

struct BuildingCreated : entityx::Event<BuildingCreated> {
    entityx::Entity entity;

    BuildingCreated(entityx::Entity entity)
        : entity(entity) {
    }
};

#endif
