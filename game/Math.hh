#ifndef STRAT_GAME_MATH_HH
#define STRAT_GAME_MATH_HH

#include <glm/glm.hpp>

#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

template <typename T>
T lerp(T a, T b, T t) {
    return (1 - t) * a + t * b;
}

float randomFloat();

struct Ray {
    glm::vec3 origin, direction;

    Ray() {
    }

    Ray(const glm::vec3 &origin, const glm::vec3 &direction)
        : origin(origin), direction(direction) {
    }
};

bool intersectTriangleWithRay(const Ray &ray,
                              const glm::vec3 &a,
                              const glm::vec3 &b,
                              const glm::vec3 &c,
                              float &t, float &u, float &v);

struct AABB {
    glm::vec3 min, max;

    AABB(const glm::vec3 &min, const glm::vec3 &max)
        : min(min), max(max) {
    }

    bool intersectWithRay(const Ray &, float t0, float t1, float *distance = NULL) const;
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
