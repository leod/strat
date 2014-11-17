#include "common/Defs.hh"

#include <algorithm>

BuildingTypeInfo buildingTypeInfo[BUILDING_MAX] = {
    { { { RESOURCE_IRON, { 0, 0, 0 } },
        { RESOURCE_IRON, { 0, 1, 0 } },
        { RESOURCE_IRON, { 1, 0, 0 } },
        { RESOURCE_IRON, { 1, 1, 0 } },
        { RESOURCE_IRON, { 1, 1, 1 } },
        { RESOURCE_IRON, { 1, 0, 1 } },
        { RESOURCE_IRON, { 0, 0, 2 } },
        { RESOURCE_IRON, { 0, 0, 3 } } } },
    { { { RESOURCE_IRON, { 0, 0, 0 } },
        { RESOURCE_IRON, { 0, 1, 0 } },
        { RESOURCE_IRON, { 1, 1, 0 } },
        { RESOURCE_IRON, { 1, 0, 0 } },
        { RESOURCE_IRON, { 0, 0, 1 } },
        { RESOURCE_IRON, { 0, 1, 1 } },
        { RESOURCE_IRON, { 1, 1, 1 } },
        { RESOURCE_IRON, { 1, 0, 1 } } } },
    { { { RESOURCE_IRON, { 0, 0, 0 } } } },
    { { { RESOURCE_IRON, { 0, 0, 0 } },
        { RESOURCE_IRON, { 0, 0, 1 } },
        { RESOURCE_IRON, { 0, 0, 2 } } } }
};

BuildingTypeInfo::BuildingTypeInfo(const std::vector<Block> &blocks)
    : size(0, 0, 0), blocks(blocks), costs{} {
    for (const auto &block : blocks) {
        if (block.pos.x > size.x) size.x = block.pos.x;
        if (block.pos.y > size.y) size.y = block.pos.y;
        if (block.pos.z > size.z) size.z = block.pos.z;

        costs[block.resource]++;
    }

    size += glm::uvec3(1, 1, 1);
}

bool operator==(const BuildingTypeInfo::Block &a, const BuildingTypeInfo::Block &b) {
    return a.resource == b.resource
           && a.pos == b.pos;
}

size_t sqDistance(const glm::uvec2 &a, const glm::uvec2 &b) {
    size_t x = std::max(a.x, b.x) - std::min(a.x, b.x);
    size_t y = std::max(a.y, b.y) - std::min(a.y, b.y);
    return x * x + y * y;
}

Fixed manhattanDistance(const fvec2 &a, const fvec2 &b) {
    return (a.x - b.x).abs() + (a.y - b.y).abs();
}

bool pointInRange(size_t range,
                  const glm::uvec2 &center, const glm::uvec2 &point) {
    return sqDistance(center, point) <= range * range;
}
