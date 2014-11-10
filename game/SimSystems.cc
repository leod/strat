#include "SimSystems.hh"

#include "SimState.hh"
#include "SimComponents.hh"

void MinerBuildingSystem::tick(SimState &state) {
    GameObject::Handle gameObject;
    Building::Handle building;
    MinerBuilding::Handle miner;
    for (auto entity : state.entities.entities_with_components(gameObject, building, miner)) {
        if (miner->amountStored >= Fixed(2)) {
            //std::cout << "WHEEEEEEEE... " << gameObject->getId() << std::endl;
            
            entityx::Entity target = state.findClosestBuilding(BUILDING_STORE, gameObject->getOwner(),
                                                               glm::uvec2(building->getPosition()),
                                                               100);
            if (target) {
                //std::cout << "Sending off to " << target.component<GameObject>()->getId() << std::endl;

                state.addResourceTransfer(entity, target, RESOURCE_IRON, miner->amountStored.toInt()); 

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

            if (flyingObject->progress > Fixed(1)) {
                state.events.emit<FlyingObjectLanded>(entity);
                state.entities.destroy(entity.id());
                continue;
            }
        }
    }
}

void FlyingResourceSystem::configure(entityx::EventManager &events) {

}

void FlyingResourceSystem::receive(const FlyingObjectLanded &event) {
    if (auto resource = event.entity.component<FlyingResource>()) {
            
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
        if (gp.water == Fixed(0)) {
            map.crater(p, 2);

            map.forNeighbors(p, [&] (GridPoint &gp2) {
                if (gp2.water > Fixed(0))
                    gp2.water /= Fixed(5);
                map.crater(gp2.pos, 1);
            });
        } else {
            gp.water /= Fixed(7);
            map.forNeighbors(p, [&] (GridPoint &gp2) {
                gp2.water /= Fixed(5);
            });
        }
    }
}
