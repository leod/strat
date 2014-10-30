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

void ResourceTransferSystem::tick(SimState &state) {
    Fixed distancePerSecond(5);

    ResourceTransfer::Handle resourceTransfer;
    for (auto entity : state.entities.entities_with_components(resourceTransfer)) {
        if (resourceTransfer->progress <= Fixed(1)) { 
            resourceTransfer->lastProgress = resourceTransfer->progress;

            Fixed progressPerSecond = distancePerSecond / resourceTransfer->distance;
            resourceTransfer->progress += state.getTickLengthS() * progressPerSecond;

            if (resourceTransfer->progress > Fixed(1)) {
                state.entities.destroy(entity.id());
                continue;
            }
        }

        //std::cout << (float)resourceTransfer->progress << std::endl;
    }
}
