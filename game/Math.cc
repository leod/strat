#include "Math.hh"

#include <cstdlib>
#include <iostream>

bool AABB::intersectWithRay(const Ray &ray, float *distance) const {
    glm::vec3 tMin((min - ray.origin) / ray.direction),
              tMax((max - ray.origin) / ray.direction),
              t1(glm::min(tMin, tMax)),
              t2(glm::max(tMin, tMax));

    float tNear(glm::max(glm::max(t1.x, t1.y), t1.z)),
          tFar(glm::min(glm::min(t2.x, t2.y), t2.z));

    if (tNear > tFar)
        return false;
    if (tNear < 0.0001f) {
        if (distance != NULL)
            *distance = tFar;

        return true;
    }

    return false;
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
