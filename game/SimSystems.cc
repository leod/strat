#include "SimSystems.hh"

#include "SimState.hh"
#include "SimComponents.hh"

void MinerBuildingSystem::tick(SimState &state) {
    GameObject::Handle gameObject;
    Building::Handle building;
    MinerBuilding::Handle miner;
    for (auto entity : state.entities.entities_with_components(gameObject, building, miner)) {
        if (!building->isFinished())
            continue;

        if (miner->amountStored >= Fixed(2)) {
            // Launch off resources gathered to store

            entityx::Entity target =
                    state.findClosestBuilding(BUILDING_STORE, gameObject->getOwner(),
                                              glm::uvec2(building->getPosition()),
                                              100);
            if (target) {
                state.addFlyingResource(entity, target, RESOURCE_IRON,
                                        miner->amountStored.toInt()); 

                miner->amountStored = 0;
            }
        } else {
            miner->amountStored += state.getTickLengthS(); // 1 per second
        }
    }
}

void FlyingObjectSystem::tick(SimState &state) {
    Fixed distancePerSecond(5);

    FlyingObject::Handle flyingObject;
    for (auto entity : state.entities.entities_with_components(flyingObject)) {
        if (flyingObject->progress <= Fixed(1)) { 
            flyingObject->lastProgress = flyingObject->progress;

            Fixed progressPerSecond = distancePerSecond / flyingObject->distance;
            flyingObject->progress += state.getTickLengthS() * progressPerSecond;

            // Flying object arrived?
            if (flyingObject->progress > Fixed(1)) {
                state.events.emit<FlyingObjectLanded>(entity);
                state.entities.destroy(entity.id());
                continue;
            }
        }
    }
}

void FlyingResourceSystem::configure(entityx::EventManager &events) {
    events.subscribe<FlyingObjectLanded>(*this);
}

void FlyingResourceSystem::receive(const FlyingObjectLanded &event) {
    auto entity(event.entity);
    auto flyingObject(entity.component<FlyingObject>());
    assert(flyingObject);

    if (auto resource = entity.component<FlyingResource>()) {
        auto position = glm::uvec2(flyingObject->toPosition.x.toInt(),
                                   flyingObject->toPosition.y.toInt());

        if (simState.getMap().isPoint(position) && simState.getMap().point(position).entity) {
            auto entity = simState.getMap().point(position).entity;
            auto building = entity.component<Building>();

            if (building->getType() == BUILDING_STORE) {
                auto player = entity.component<GameObject>()->getOwner();

                simState.getPlayer(player).giveResources(resource->resource, resource->amount);
            }
        }
    }
}

void FlyingBlockSystem::configure(entityx::EventManager &events) {
    events.subscribe<FlyingObjectLanded>(*this);
}

void FlyingBlockSystem::receive(const FlyingObjectLanded &event) {
    auto entity(entities.get(event.entity.id()));
    auto flyingObject(entity.component<FlyingObject>());
    assert(flyingObject);

    if (auto flyingBlock = entity.component<FlyingBlock>()) {
        if (!flyingBlock->targetEntity)
            return;

        Building::Handle targetBuilding = flyingBlock->targetEntity.component<Building>();
        assert(targetBuilding);

        if (targetBuilding->isFinished())
            return;

        targetBuilding->addBlock(flyingBlock->block);
    }
}

void RocketSystem::configure(entityx::EventManager &events) {
    events.subscribe<FlyingObjectLanded>(*this);
}

void RocketSystem::receive(const FlyingObjectLanded &event) {
    auto flyingObject(event.entity.component<FlyingObject>());
    assert(flyingObject);

    if (auto rocket = event.entity.component<Rocket>()) {
        Map::Pos p(flyingObject->toPosition);
        assert(map.isPoint(p));
        GridPoint &gp(map.point(p));

        // Water protects the terrain 
        //if (gp.water == Fixed(0)) {
            map.crater(p, 2);

            map.forNeighbors(p, [&] (GridPoint &gp2) {
                //if (gp2.water > Fixed(0))
                    //gp2.water /= Fixed(5);
                map.crater(gp2.pos, 1);
            });
        //} else {
            Fixed k(Fixed(17)/Fixed(12));
            gp.water /= k;
            //gp.water /= Fixed(7);
            map.forNeighbors(p, [&] (GridPoint &gp2) {
                gp2.water /= Fixed(14)/Fixed(12);

                map.forNeighbors(p, [&] (GridPoint &gp3) {
                    gp3.water /= Fixed(13)/Fixed(12);
                });
            });
        //}
    }
}

void MainBuildingSystem::tick(SimState &state) {
    Building::Handle building;
    MainBuilding::Handle mainBuilding;
    for (auto entity : state.entities.entities_with_components(building, mainBuilding)) {
        if (!building->isFinished()) continue; 

        if (!mainBuilding->buildQueue.empty()) {
            entityx::Entity buildEntity = mainBuilding->buildQueue.front();

            if (!buildEntity) {
                mainBuilding->removeFromQueue(buildEntity);
                continue;
            }

            auto buildBuilding = buildEntity.component<Building>();

            if (buildBuilding->isFinished() || !buildBuilding->isBlockNeeded()) {
                mainBuilding->removeFromQueue(buildEntity);
                continue;
            }

            // Send off resource block to building being built
            Fixed timeSinceLastLaunch = state.getTimeS() - mainBuilding->timeLastLaunch;
            if (timeSinceLastLaunch >= mainBuilding->getBuildSpeed()) {
                mainBuilding->timeLastLaunch = state.getTimeS(); 

                auto flying = state.addFlyingBlock(entity, buildEntity,
                                                   buildBuilding->nextNeededBlock());

                buildBuilding->incomingBlocks.push_back(flying.component<FlyingBlock>());
            }
        }
    }
}
