#ifndef STRAT_COMMON_DEFS_HH
#define STRAT_COMMON_DEFS_HH

#include <cstdint>
#include <cstddef>

typedef uint16_t PlayerId; // Valid Ids are > 0
typedef uint32_t ObjectId; // Likewise

enum BuildingType {
    BUILDING_BASE,
    BUILDING_STORE,
    BUILDING_MINER,
    BUILDING_MAX
};

enum ResourceType {
    RESOURCE_IRON,
    RESOURCE_MAX
};

struct BuildingTypeInfo {
    size_t sizeX;
    size_t sizeY; 
    size_t sizeZ;
};

// Might be made dynamic later and a part of Sim
extern BuildingTypeInfo buildingTypeInfo[BUILDING_MAX];

#endif
