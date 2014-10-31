#ifndef STRAT_GAME_MATH_HH
#define STRAT_GAME_MATH_HH

#include <glm/glm.hpp>

#include <vector>

template <typename T>
T lerp(T a, T b, T t) {
    return (1 - t) * a + t * b;
}

float randomFloat();

struct Ray {
    glm::vec3 origin, direction;

    Ray(const glm::vec3 &origin, const glm::vec3 &direction)
        : origin(origin), direction(direction) {
    }
};

struct AABB {
    glm::vec3 min, max;

    AABB(const glm::vec3 &min, const glm::vec3 &max)
        : min(min), max(max) {
    }

    bool intersectWithRay(const Ray &, float *distance = NULL) const;
};

struct PerlinNoise {
    PerlinNoise(size_t width, size_t height);

    void generate(size_t octaves, float persistence = 0.5f);
    void smooth();

    float &get(size_t x, size_t y) {
        return noise[y*width + x];
    }

    const float &get(size_t x, size_t y) const {
        return noise[y*width + x];
    }

private:
    size_t width;
    size_t height;

    std::vector<float> whiteNoise;
    std::vector<float> noise;

    void generateWhiteNoise();
    float generateSmoothNoise(size_t octave, size_t x, size_t y);
};

#endif
