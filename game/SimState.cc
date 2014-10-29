#include "SimState.hh"

#include "SimComponents.hh"

#include <cstdlib>

SimState::SimState(const GameSettings &settings)
    : settings(settings)
    , map(Map::generate(settings.mapW, settings.mapH,
                        settings.heightLimit,
                        settings.randomSeed)) {

    // Place random spawn points for now...
    for (auto &player : settings.players) {
        size_t x, y;
        do {
            x = rand() % settings.mapW;
            y = rand() % settings.mapH;
        } while (!canPlaceBuilding(BUILDING_BASE, x, y));

        Order order(Order::BUILD);
        order.player = player.id;
        order.build.type = BUILDING_BASE;
        order.build.x = x;
        order.build.y = y;

        assert(isOrderValid(order));

        runOrder(order);
    }
}

bool SimState::canPlaceBuilding(BuildingType type, size_t px, size_t py) const {
    assert(type >= 0 && type < BUILDING_MAX);

    if (px >= map.getSizeX()
        || py >= map.getSizeY()) {
        return false;
    }

    const BuildingTypeInfo &typeInfo(buildingTypeInfo[type]);
    bool valid = true;

    size_t height = map.point(px, py).height;

    for (size_t x = px; x < px + typeInfo.sizeX; x++) {
        if (x >= map.getSizeX()) {
            valid = false;
            break;
        }
        for (size_t y = py; y < py + typeInfo.sizeY; y++) {
            if (y >= map.getSizeY()) {
                valid = false;
                break;
            }
            if (map.point(x, y).entity
                || map.point(x, y).height != height) {
                valid = false;
                break;
            }
        }
    }

    return valid;
}

bool SimState::isOrderValid(const Order &order) const {
    switch (order.type) {
    case Order::BUILD: {
        return canPlaceBuilding(order.build.type, order.build.x, order.build.y);
    }
    default:
        return false;
    }
}

void SimState::runOrder(const Order &order) {
    assert(isOrderValid(order));

    switch (order.type) {
    case Order::BUILD: {
        std::cout << "Got build order at x=" << order.build.x
                  << ", y=" << order.build.y 
                  << ", by " << order.player << std::endl;

        entityx::Entity entity = entities.create();
        entity.assign<GameObject>(order.player, ++entityCounter);
        entity.assign<Building>(order.build.type,
            order.build.x, order.build.y);

        if (order.build.type == BUILDING_MINER)
            entity.assign<MinerBuilding>(RESOURCE_IRON);

        events.emit<BuildingCreated>(entity);

        const BuildingTypeInfo &type(buildingTypeInfo[order.build.type]);
        for (size_t x = order.build.x; x < order.build.x + type.sizeX; x++) {
            for (size_t y = order.build.y; y < order.build.y + type.sizeY; y++) {
                map.point(x, y).entity = entity;
            }
        }

        return;
    }

    default:
        return;
    }
}
