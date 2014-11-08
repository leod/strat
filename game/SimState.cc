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
      entityCounter(0),
      time(0),
      waterLevel(0) {
    // Place random spawn points for now...
    for (auto &player : settings.players) {
        size_t x, y;
        do {
            x = rand() % settings.mapW;
            y = rand() % settings.mapH;
        } while (!canPlaceBuilding(BUILDING_BASE, glm::uvec2(x, y)));

        Order order(Order::BUILD);
        order.player = player.id;
        order.build.type = BUILDING_BASE;
        order.build.x = x;
        order.build.y = y;

        assert(isOrderValid(order));

        runOrder(order);
    }

    // Generate some trees
    size_t numTrees = 20;

    for (size_t i = 0; i < numTrees; i++) {
        size_t x, y;
        do {
            x = rand() % settings.mapW;
            y = rand() % settings.mapH;
        } while (!map.point(x, y).usable());

        placeTree(Map::Pos(x, y));
    }
}

bool SimState::canPlaceBuilding(BuildingType type, const glm::uvec2 &p) const {
    assert(type >= 0 && type < BUILDING_MAX);

    if (!map.isPoint(p)) return false;

    const BuildingTypeInfo &typeInfo(buildingTypeInfo[type]);

    size_t height = map.point(p).height;

    for (size_t x = p.x > 0 ? p.x - 1 : 0; x < p.x + typeInfo.size.x + 1; x++) {
        if (x >= map.getSizeX()) {
            return false;
        }
        for (size_t y = p.y > 0 ? p.y - 1 : 0; y < p.y + typeInfo.size.y + 1; y++) {
            if (y >= map.getSizeY()) {
                return false;
            }
            if (map.point(x, y).height != height) { // Map is not even on our rect
                return false;
            }
            if (!map.point(x, y).usable()) { // Grid point is being used right now
                return false;
            }
        }
    }

    return true;
}

entityx::Entity SimState::findClosestBuilding(BuildingType type,
                                              PlayerId owner,
                                              const glm::uvec2 &p,
                                              size_t maxRange) const {
    entityx::Entity entity;
    size_t minDistance = 0;

    size_t startX = p.x >= maxRange ? p.x - maxRange : 0;
    size_t startY = p.y >= maxRange ? p.y - maxRange : 0;

    for (size_t x = startX; x < p.x + maxRange && x < map.getSizeX(); x++) {
        for (size_t y = startY; y < p.y + maxRange && y < map.getSizeY(); y++) {
            const GridPoint &point(map.point(x, y));

            if (point.entity.valid()
                && point.entity.has_component<Building>()
                && point.entity.component<Building>()->getType() == type
                && point.entity.component<GameObject>()->getOwner() == owner) {
                size_t distance = sqDistance(p, glm::uvec2(x, y));

                if (distance <= maxRange * maxRange
                    && (!entity.valid() || distance < minDistance)) {
                    entity = point.entity;
                    minDistance = distance;
                }
            }
        }
    }

    return entity;
}

void SimState::placeTree(const glm::uvec2 &p) {
    assert(map.point(p).usable());

    entityx::Entity entity = entities.create();
    entity.assign<GameObject>(PLAYER_NEUTRAL, ++entityCounter);
    entity.assign<Tree>(glm::uvec3(p.x, p.y, map.point(p.x, p.y).height));

    map.point(p).entity = entity;
}

bool SimState::isOrderValid(const Order &order) const {
    switch (order.type) {
    case Order::BUILD:
        return canPlaceBuilding(order.build.type,
                                glm::uvec2(order.build.x, order.build.y));
    case Order::RAISE_MAP: {
        if(!map.isPoint(order.raiseMap.x, order.raiseMap.y)
           || !map.isPoint(order.raiseMap.x + order.raiseMap.w,
                           order.raiseMap.y + order.raiseMap.h))
            return false;

        bool valid = true;
        map.forRectangle(Map::Pos(order.raiseMap.x, order.raiseMap.y),
                         Map::Pos(order.raiseMap.w, order.raiseMap.h),
                         [&] (const GridPoint &p) {
                             valid = valid && p.usable();
                         });
        return valid;
    }
    
    case Order::ATTACK: {
        if (!map.isPoint(order.attack.x, order.attack.y))
            return false;

        entityx::Entity entity = getGameObject(order.attack.objectId);
        if (!entity)
            return false;

        GameObject::Handle gameObject(entity.component<GameObject>());
        assert(gameObject);
        if (gameObject->getOwner() != order.player)
            return false;

        Building::Handle building(entity.component<Building>());
        return building && building->getType() == BUILDING_TOWER;
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
            glm::uvec3(order.build.x, order.build.y,
                       map.point(order.build.x, order.build.y).height));

        if (order.build.type == BUILDING_MINER)
            entity.assign<MinerBuilding>(RESOURCE_IRON);

        events.emit<BuildingCreated>(entity);

        const BuildingTypeInfo &type(buildingTypeInfo[order.build.type]);
        for (size_t x = order.build.x; x < order.build.x + type.size.x; x++) {
            for (size_t y = order.build.y; y < order.build.y + type.size.y; y++) {
                map.point(x, y).entity = entity;
            }
        }

        return;
    }

    case Order::RAISE_MAP: {
        std::cout << "Got raise map order at x=" << order.raiseMap.x
                  << ", y=" << order.raiseMap.y
                  << ", w=" << order.raiseMap.w
                  << ", h=" << order.raiseMap.h << std::endl;
        map.raise(Map::Pos(order.raiseMap.x, order.raiseMap.y),
                  Map::Pos(order.raiseMap.w, order.raiseMap.h));
        return;
    }

    case Order::ATTACK: {
        std::cout << "Got attack order at x=" << order.attack.x
                  << ", y=" << order.attack.y
                  << ", from=" << order.attack.objectId << std::endl;

        entityx::Entity entity = getGameObject(order.attack.objectId);

        // TODO: Random numbers again..
        int radius = 10;
        int dx = rand() % radius - radius / 2;
        int dy = rand() % radius - radius / 2;

        int x = order.attack.x + dx;
        int y = order.attack.y + dy;

        if (x < 0) x = 0;
        if (x >= map.getSizeX()) x = map.getSizeX() - 1;
        if (y < 0) y = 0;
        if (y >= map.getSizeY()) y = map.getSizeY() - 1;

        addRocket(entity, Map::Pos(x, y));
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

entityx::Entity SimState::getGameObject(ObjectId id) const {
    entityx::Entity result;
    GameObject::Handle gameObject;

    auto ents = const_cast<entityx::EntityManager *>(&entities); // I'm sorry...

    // TODO: Maybe switch to a std::map<ObjectId, Entity> at some point
    for (auto entity : ents->entities_with_components(gameObject)) {
        if (gameObject->getId() == id) {
            assert(!result);
            result = entity;
        }
    }

    return result;
}


Fixed SimState::getTickLengthS() const {
    return Fixed(settings.tickLengthMs) / Fixed(1000);
}

void SimState::addResourceTransfer(Entity fromEntity, Entity toEntity,
                                   ResourceType resource,
                                   size_t amount) {
    Building::Handle fromBuilding(fromEntity.component<Building>()),
                     toBuilding(toEntity.component<Building>());
    GameObject::Handle fromObject(fromEntity.component<GameObject>());
    assert(fromBuilding && toBuilding && fromObject);

    // For now: start and end in the middle of the top of the buildings
    // These positions are used to calculate the distance of the transfer,
    // and for rendering.
    fvec3 fromPosition(fromBuilding->getPosition()),
          toPosition(toBuilding->getPosition());

    BuildingTypeInfo &fromTi(fromBuilding->getTypeInfo()),
                     &toTi(toBuilding->getTypeInfo());

    fromPosition += fvec3(Fixed(fromTi.size.x) / Fixed(2),
                          Fixed(fromTi.size.y) / Fixed(2),
                          Fixed(fromTi.size.z));
    toPosition += fvec3(Fixed(toTi.size.x) / Fixed(2),
                        Fixed(toTi.size.y) / Fixed(2),
                        Fixed(toTi.size.z));

    entityx::Entity entity = entities.create();
    entity.assign<GameObject>(fromObject->getOwner(), ++entityCounter);
    entity.assign<FlyingObject>(fromPosition, toPosition);
    entity.assign<FlyingResource>(resource, amount);
}

void SimState::addRocket(Entity fromEntity, Map::Pos toPos) {
    Building::Handle fromBuilding(fromEntity.component<Building>());
    GameObject::Handle fromObject(fromEntity.component<GameObject>());
    assert(fromBuilding && fromObject);

    fvec3 fromPosition(fromBuilding->getPosition()),
          toPosition(toPos, map.point(toPos).height);

    BuildingTypeInfo &fromTi(fromBuilding->getTypeInfo());
    fromPosition += fvec3(Fixed(fromTi.size.x) / Fixed(2),
                          Fixed(fromTi.size.y) / Fixed(2),
                          Fixed(fromTi.size.z));

    entityx::Entity entity = entities.create();
    entity.assign<GameObject>(fromObject->getOwner(), ++entityCounter);
    entity.assign<FlyingObject>(fromPosition, toPosition);
    entity.assign<Rocket>();
}

void SimState::raiseWaterLevel() {
    std::cout << "Raising water level" << std::endl;

    waterLevel++;

    if (waterLevel == 1) {
        for (size_t x = 0; x < map.getSizeX(); x++) {
            for (size_t y = 0; y < map.getSizeY(); y++) {
                if (map.point(x, y).height == 0)
                    map.point(x, y).water = true;
            }
        }
    }
}

void SimState::waterTick() {
    time += getTickLengthS();
    if ((size_t)time.toInt() / 5 >= waterLevel && waterLevel < map.getMaxHeight()) {
        //raiseWaterLevel();
    }

    Fixed flowPerS(Fixed(4) / Fixed(6));

    // Calculate water flow from grid point to grid point
    for (size_t x = 0; x < map.getSizeX(); x++) {
        for (size_t y = 0; y < map.getSizeY(); y++) {
            GridPoint &p(map.point(x, y));

            if (p.height < waterLevel) {
                if (p.water < Fixed(1)) {
                    // This point has only been flooded since the new water level

                    Fixed sum;
                    map.forNeighbors(Map::Pos(x, y), [&] (const GridPoint &p) {
                        Fixed value(p.water);

                        if (value < p.water) return;
                        if (value > Fixed(1)) value = Fixed(1);

                        //value *= Fixed(p.water) - value;

                        sum += value * flowPerS * getTickLengthS();
                    });

                    p.water += sum;
                } else if (p.water < Fixed(waterLevel - p.height)) {
                    // This point has been flooded for a longer time,
                    // but hasn't risen to the waterLevel yet

                    p.water += Fixed(1) / Fixed(4 * (waterLevel - p.height));
                }

                // Don't allow flooding above the global water level
                if (p.water > Fixed(waterLevel - p.height))
                    p.water = Fixed(waterLevel - p.height);
            }
        }
    }
}

SimState::PlayerMap SimState::playersFromSettings(const GameSettings &settings) {
    SimState::PlayerMap players;

    for (auto info : settings.players) {
        players.emplace(info.id, info);
    }

    return players;
}
