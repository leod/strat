#include "SimState.hh"

#include "SimComponents.hh"

#include <cstdlib>

PlayerState::PlayerState(const PlayerInfo &info)
    : info(info), resources(RESOURCE_MAX, 0) {
}

SimState::SimState(const GameSettings &settings)
    : settings(settings),
      map(Map::generate(settings.mapW, settings.mapH,
                        settings.heightLimit,
                        settings.randomSeed)),
      players(playersFromSettings(settings)),
      entityCounter(0) {
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

    for (size_t x = px > 0 ? px - 1 : 0; x < px + typeInfo.sizeX + 1; x++) {
        if (x >= map.getSizeX()) {
            valid = false;
            break;
        }
        for (size_t y = py > 0 ? py - 1 : 0; y < py + typeInfo.sizeY + 1; y++) {
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

entityx::Entity SimState::findClosestBuilding(BuildingType type,
                                              PlayerId owner,
                                              size_t px, size_t py,
                                              size_t maxRange) const {
    entityx::Entity entity;
    size_t minDistance = 0;

    size_t startX = px >= maxRange ? px - maxRange : 0;
    size_t startY = py >= maxRange ? py - maxRange : 0;

    for (size_t x = startX; x < startX + maxRange && x < map.getSizeX(); x++) {
        for (size_t y = startY; y < startY + maxRange && y < map.getSizeY(); y++) {
            const GridPoint &point(map.point(x, y));

            if (point.entity.valid()
                && point.entity.has_component<Building>()
                && point.entity.component<Building>()->getType() == type
                && point.entity.component<GameObject>()->getOwner() == owner) {
                size_t distance = (std::max(px, x) - std::min(px, x)) * (std::max(px, x) - std::min(px, x)) +
                                  (std::max(py, y) - std::min(py, y)) * (std::max(py, y) - std::min(py, y));
                if (distance <= maxRange * maxRange && (!entity.valid() || distance < minDistance)) {
                    entity = point.entity;
                    minDistance = distance;
                }
            }
        }
    }

    return entity;
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

PlayerState &SimState::getPlayer(PlayerId id) {
    auto it = players.find(id);
    assert(it != players.end());
    return it->second;
}

const PlayerState &SimState::getPlayer(PlayerId id) const {
    auto it = players.find(id);
    assert(it != players.end());
    return it->second;
}

Fixed SimState::getTickLengthS() const {
    return settings.tickLengthMs / 1000.0f;
}

void SimState::addResourceTransfer(Entity from, Entity to,
                                   ResourceType resource,
                                   size_t amount) {
    Building::Handle fromBuilding(from.component<Building>()),
                     toBuilding(to.component<Building>());

    Fixed fromX = Fixed((int)fromBuilding->getX()) + Fixed((int)fromBuilding->getTypeInfo().sizeX) / Fixed(2);
    Fixed fromY = Fixed((int)fromBuilding->getY()) + Fixed((int)fromBuilding->getTypeInfo().sizeY) / Fixed(2);
    Fixed fromZ = Fixed((int)map.point(fromBuilding->getX(), fromBuilding->getY()).height);
    Fixed toX = Fixed((int)toBuilding->getX()) + Fixed((int)toBuilding->getTypeInfo().sizeX) / Fixed(2);
    Fixed toY = Fixed((int)toBuilding->getY()) + Fixed((int)toBuilding->getTypeInfo().sizeY) / Fixed(2);
    Fixed toZ = Fixed((int)map.point(toBuilding->getX(), toBuilding->getY()).height);

    /*std::cout << (float)fromX << std::endl;
    std::cout << (float)fromY << std::endl;
    std::cout << (float)toX << std::endl;
    std::cout << (float)toY << std::endl << std::endl;
    std::cout << (int)fromX << std::endl;
    std::cout << (int)fromY << std::endl;
    std::cout << (int)toX << std::endl;
    std::cout << (int)toY << std::endl;*/

    entityx::Entity entity = entities.create();
    entity.assign<GameObject>(PLAYER_NEUTRAL, ++entityCounter);
    entity.assign<ResourceTransfer>(from, to,
                                    fromX, fromY, fromZ,
                                    toX, toY, toZ,
                                    resource,
                                    amount);
}

SimState::PlayerMap SimState::playersFromSettings(const GameSettings &settings) {
    SimState::PlayerMap players;

    for (auto info : settings.players) {
        players.emplace(info.id, info);
    }

    return players;
}
