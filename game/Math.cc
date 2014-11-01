#include "Math.hh"

#include <cstdlib>
#include <iostream>
#include <limits>

bool intersectTriangleWithRay(const Ray &ray,
                              const glm::vec3 &a,
                              const glm::vec3 &b,
                              const glm::vec3 &c,
                              float &t,
                              float &u,
                              float &v) {
    glm::vec3 edge1 = b - a;
    glm::vec3 edge2 = c - a;
    glm::vec3 pvec = glm::cross(ray.direction, edge2);
    float det = glm::dot(edge1, pvec);

    if (det > 0.0000001 && det < 0.0000001)
        return false;

    float invDet = 1.0 / det;

    glm::vec3 tvec = ray.origin - a;

    u = glm::dot(tvec, pvec) * invDet;
    if (u < 0 || u > 1)
        return false;
        
    glm::vec3 qvec = glm::cross(tvec, edge1);

    v = glm::dot(ray.direction, qvec) * invDet;
    if (v < 0 || u + v > 1)
        return false;
        
    t = glm::dot(edge2, qvec) * invDet;

    return t > 0.0000001;
}

bool AABB::intersectWithRay(const Ray &ray, float t0, float t1, float *distance) const {
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    glm::vec3 inv_direction = 1.0f / ray.direction;
    glm::vec3 parameters[2] = { min, max };

    bool sign[3] = { inv_direction.x < 0,
                     inv_direction.y < 0,
                     inv_direction.z < 0 };

    tmin = (parameters[sign[0]].x - ray.origin.x) * inv_direction.x;
    tmax = (parameters[1-sign[0]].x - ray.origin.x) * inv_direction.x;
    tymin = (parameters[sign[1]].y - ray.origin.y) * inv_direction.y;
    tymax = (parameters[1-sign[1]].y - ray.origin.y) * inv_direction.y;
    if ((tmin > tymax) || (tymin > tmax)) 
            return false;
    if (tymin > tmin)
            tmin = tymin;
    if (tymax < tmax)
            tmax = tymax;
    tzmin = (parameters[sign[2]].z - ray.origin.z) * inv_direction.z;
    tzmax = (parameters[1-sign[2]].z - ray.origin.z) * inv_direction.z;
    if ((tmin > tzmax) || (tzmin > tmax)) 
            return false;
    if (tzmin > tmin)
            tmin = tzmin;
    if (tzmax < tmax)
            tmax = tzmax;
    return ((tmin < t1) && (tmax > t0));
}

float randomFloat() {
    return static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
}

PerlinNoise::PerlinNoise(size_t width, size_t height)
    : width(width)
    , height(height)
    , whiteNoise(width * height)
    , noise (width * height) {
}

void PerlinNoise::generate(size_t octaves, float persistence) {
    assert(octaves > 0);

    generateWhiteNoise();

    float amplitude = 1.0f;
    float totalAmplitude = 0.0f; 

    for (int octave = octaves - 1; octave >= 0; octave--) {
        amplitude *= persistence;
        totalAmplitude += amplitude;

        for (size_t x = 0; x < width; x++) {
            for (size_t y = 0; y < height; y++) {
                noise[y*width + x] += generateSmoothNoise(octave, x, y) * amplitude;
            }
        }
    }


    for (size_t x = 0; x < width; x++) {
        for (size_t y = 0; y < height; y++) {
            noise[y*width + x] /= totalAmplitude;
        }
    }
}

void PerlinNoise::smooth() {
    std::vector<float> newNoise(width * height);
    
    for (size_t x = 0; x < width; x++) {
        for (size_t y = 0; y < height; y++) {
            float sum = noise[y*width + x];
            float n = 0;
            if (x > 0) { 
                sum += noise[y*width + (x-1)];
                n++;
            }
            if (x < width-1) {
                sum += noise[y*width + x+1];
                n++;
            }
            if (y > 0) {
                sum += noise[(y-1)*width + x];
                n++;
            }
            if (y < height-1) {
                sum += noise[(y+1)*width + x];
                n++;
            }

            newNoise[y*width + x] = sum / n;
        }
    }

    noise = newNoise;
}

void PerlinNoise::generateWhiteNoise() {
    for (size_t x = 0; x < width; x++) {
        for (size_t y = 0; y < height; y++) {
            whiteNoise[y*width + x] = randomFloat();
        }
    }
}

float PerlinNoise::generateSmoothNoise(size_t octave, size_t x, size_t y) {
    size_t samplePeriod = 1 << octave; // 2^octave
    float sampleFrequency = 1.0f / samplePeriod;

    // Largest multiple of 2^octave smaller than x
    int sample_x0 = (x / samplePeriod) * samplePeriod;
    int sample_x1 = (sample_x0 + samplePeriod) % width;

    int sample_y0 = (y / samplePeriod) * samplePeriod;
    int sample_y1 = (sample_y0 + samplePeriod) % height;

    float horizontalBlend = (x - sample_x0) * sampleFrequency;
    float verticalBlend = (y - sample_y0) * sampleFrequency;

    float top = lerp(whiteNoise[sample_y0*width + sample_x0],
                     whiteNoise[sample_y0*width + sample_x1],
                     horizontalBlend);
    float bottom = lerp(whiteNoise[sample_y1*width + sample_x0],
                        whiteNoise[sample_y1*width + sample_x1],
                        horizontalBlend);

    return lerp(top, bottom, verticalBlend);
}
