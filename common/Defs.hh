#ifndef STRAT_COMMON_DEFS_HH
#define STRAT_COMMON_DEFS_HH

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <Fixed.hh>

typedef uint16_t PlayerId; // Valid client ids are > 0
typedef uint32_t ObjectId; // Valid object ids are > 0

enum {
    PLAYER_NEUTRAL = 0
};

enum BuildingType {
    BUILDING_MAIN,
    BUILDING_STORE,
    BUILDING_MINER,
    BUILDING_TOWER,

    BUILDING_MAX
};

enum ResourceType {
    RESOURCE_IRON,

    RESOURCE_MAX
};

typedef std::array<size_t, RESOURCE_MAX> Resources;

struct BuildingTypeInfo {
    struct Block {
        ResourceType resource;
        glm::uvec3 pos; // relative to building position
    };

    glm::uvec3 size;
    std::vector<Block> blocks;
    Resources costs;

    BuildingTypeInfo(const std::vector<Block> &);
};

bool operator==(const BuildingTypeInfo::Block &, const BuildingTypeInfo::Block &);

typedef glm::detail::tvec3<Fixed, glm::highp> fvec3;
typedef glm::detail::tvec3<Fixed, glm::highp> fvec2;

size_t sqDistance(const glm::uvec2 &a, const glm::uvec2 &b);
Fixed manhattanDistance(const fvec2 &a, const fvec2 &b);

bool pointInRange(size_t range,
                  const glm::uvec2 &center, const glm::uvec2 &point);

// Might be made dynamic later and a part of Sim
extern BuildingTypeInfo buildingTypeInfo[BUILDING_MAX];

#endif
