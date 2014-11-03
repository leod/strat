#pragma once

#include <iostream>
#include <tuple>
#include <functional>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

template<typename... Args>
std::ostream& operator<<(std::ostream& os, glm::vec2 const& v) {
    return os << "vec2(" << v.x << ", "
                         << v.y << ")";
}

template<typename... Args>
std::ostream& operator<<(std::ostream& os, glm::vec3 const& v) {
    return os << "vec3(" << v.x << ", "
                         << v.y << ", "
                         << v.z << ")";
}

inline
std::ostream& operator<<(std::ostream& os, std::vector<uint8_t> const&) {
    return os << "<data>";
}

std::string demangle(std::string const&, bool qualified = false);
