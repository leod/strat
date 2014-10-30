#ifndef STRAT_COMMON_DEFS_HH
#define STRAT_COMMON_DEFS_HH

#include <cstdint>
#include <cstddef>

#include <glm/glm.hpp>
#include <Fixed.hh>

typedef uint16_t PlayerId; // Valid client ids are > 0
typedef uint32_t ObjectId; // Valid object ids are > 0

enum {
    PLAYER_NEUTRAL
};

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
    glm::uvec3 size;
};

typedef glm::detail::tvec3<Fixed, glm::highp> fvec3;
typedef glm::detail::tvec3<Fixed, glm::highp> fvec2;

size_t sqDistance(const glm::uvec2 &a, const glm::uvec2 &b);
Fixed manhattanDistance(const fvec2 &a, const fvec2 &b);

// Might be made dynamic later and a part of Sim
extern BuildingTypeInfo buildingTypeInfo[BUILDING_MAX];

#endif
