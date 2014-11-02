#include "Terrain.hh"
#include "Math.hh"

#include <limits>
#include <iostream>
#include <cmath>
#include <GL/glu.h>

#define POINT(x,y) glm::vec3(x, y, map.point(x, y).height + map.point(x,y).growthProgress)

struct Vertex {
    glm::vec3 position, color, normal;

    Vertex(glm::vec3 position,
           glm::vec3 color,
           glm::vec3 normal)
        : position(position),
          color(color),
          normal(normal) {
    }
};

struct TerrainPatch {
    TerrainPatch(const Map &map,
                 const Map::Pos &position,
                 const Map::Pos &size); 

    void init();
    void update();
    void draw();

    bool intersectWithRay(const Ray &ray, Map::Pos &point, float &t) const;

private:
    const Map &map;
    Map::Pos position, size;

    AABB aabb;

    glm::vec3 color(size_t height) const; 

    std::vector<Vertex> vertices;
    GLuint vertexBuffer;
};

TerrainPatch::TerrainPatch(const Map &map,
                           const Map::Pos &position,
                           const Map::Pos &size)
    : map(map), position(position), size(size),
      aabb(glm::vec3(position, 0), glm::vec3(position + size, 0)) {
    init();
}

TerrainMesh::~TerrainMesh() {
    for (auto patch : patches)
        delete patch;
}

void TerrainPatch::init() {
    glGenBuffers(1, &vertexBuffer);
    update();
}

void TerrainPatch::update() {
    aabb.max.z = 0;
    for (size_t x = position.x; x < position.x + size.x; x++) {
        for (size_t y = position.y; y < position.y + size.y; y++) {
            if (map.point(x,y).height > aabb.max.z)
                aabb.max.z = map.point(x,y).height;
        }
    }

    vertices.clear();

    for (size_t x = position.x; x < position.x + size.x; x++) {
        for (size_t y = position.y; y < position.y + size.y; y++) {
            // Two triangles per grid point

            glm::vec3 a(POINT(x,y)), b(POINT(x+1,y)),
                      c(POINT(x,y+1)), d(POINT(x+1,y+1));

            glm::vec3 n1(glm::normalize(glm::cross(a - d, b - d)));
            glm::vec3 n2(glm::normalize(glm::cross(c - d, a - d)));

            Vertex va1(a, color(a.z), n1);
            Vertex vb1(b, color(b.z), n1);
            Vertex vc1(d, color(d.z), n1);

            Vertex va2(d, color(d.z), n2);
            Vertex vb2(c, color(c.z), n2);
            Vertex vc2(a, color(a.z), n2);

            vertices.push_back(va1);
            vertices.push_back(vb1);
            vertices.push_back(vc1);

            vertices.push_back(va2);
            vertices.push_back(vb2);
            vertices.push_back(vc2);
        }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
        &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TerrainPatch::draw() {
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer); 

    glEnableClientState(GL_VERTEX_ARRAY); // oldschool yo
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vertex),
        reinterpret_cast<const void *>(offsetof(Vertex, position)));
    glColorPointer(3, GL_FLOAT, sizeof(Vertex),
        reinterpret_cast<const void *>(offsetof(Vertex, color)));
    glNormalPointer(GL_FLOAT, sizeof(Vertex),
        reinterpret_cast<const void *>(offsetof(Vertex, normal)));

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Water test
    glShadeModel(GL_FLAT);
    glBegin(GL_TRIANGLES);
    for (size_t x = position.x; x < position.x + size.x; x++) {
        for (size_t y = position.y; y < position.y + size.y; y++) {
            glm::vec3 a(POINT(x,y)), b(POINT(x+1,y)),
                      c(POINT(x,y+1)), d(POINT(x+1,y+1));
            float dz = 0.01;
            a.z += dz + map.point(x,y).water;
            b.z += dz + map.point(x+1,y).water;
            c.z += dz + map.point(x,y+1).water;
            d.z += dz + map.point(x+1,y+1).water;

            float ca = map.point(x,y).water > 1 ? 1 : map.point(x,y).water;
            float cb = map.point(x+1,y).water > 1 ? 1 : map.point(x+1,y).water;
            float cc = map.point(x,y+1).water > 1 ? 1 : map.point(x,y+1).water;
            float cd = map.point(x+1,y+1).water > 1 ? 1 : map.point(x+1,y+1).water;

#define ALPHA(x) (0.40f + sin(x)/2)

            /*glNormal3f(0,0,1);
            glColor4f(0.0f, 0.2f, 0.5f, ALPHA(cd));
            glVertex3f(POINT(x,y).x, POINT(x,y).y, POINT(x,y).z);
            glVertex3f(POINT(x,y).x, POINT(x,y).y, a.z);
            continue;*/

            float minw = 0.1;

            if (map.point(x+1,y+1).water > minw || map.point(x+1,y).water > minw ||
                map.point(x,y).water > minw) {
                glm::vec3 n1(glm::normalize(glm::cross(a - d, b - d)));
                glNormal3f(n1.x, n1.y, n1.z);

                glColor4f(0.0f, 0.2f, 0.5f, ALPHA((cd + cb + ca) / 3));
                glVertex3f(a.x, a.y, a.z);
                //glColor4f(0.0f, 0.2f, 0.5f, ALPHA(cb));
                glVertex3f(b.x, b.y, b.z);
                glVertex3f(d.x, d.y, d.z);
                //glColor4f(0.0f, 0.2f, 0.5f, ALPHA(ca));

                /*glm::vec3 n2(glm::normalize(glm::cross(c - d, a - d)));
                glNormal3f(n2.x, n2.y, n2.z);

                glColor4f(0.0f, 0.2f, 0.5f, ALPHA((ca + cc + cd) / 3));
                glVertex3f(a.x, a.y, a.z);
                //glColor4f(0.0f, 0.2f, 0.5f, ALPHA(cc));
                glVertex3f(c.x, c.y, c.z);
                //glColor4f(0.0f, 0.2f, 0.5f, ALPHA(cd));
                glVertex3f(d.x, d.y, d.z);*/
            }
            
            if (map.point(x,y).water > minw || map.point(x,y+1).water > minw ||
                map.point(x+1,y+1).water > minw) {
                glm::vec3 n2(glm::normalize(glm::cross(c - d, a - d)));
                glNormal3f(n2.x, n2.y, n2.z);

                glColor4f(0.0f, 0.2f, 0.5f, ALPHA((ca + cc + cd) / 3));
                glVertex3f(d.x, d.y, d.z);
                //glColor4f(0.0f, 0.2f, 0.5f, ALPHA(cc));
                glVertex3f(c.x, c.y, c.z);
                glVertex3f(a.x, a.y, a.z);
                //glColor4f(0.0f, 0.2f, 0.5f, ALPHA(cd));
            }

        }
    }
    glEnd();
    glShadeModel(GL_SMOOTH);
}

bool TerrainPatch::intersectWithRay(const Ray &ray, Map::Pos &point,
                                    float &tMin) const {
    /*if (!aabb.intersectWithRay(ray, 0.1f, 5000.0f))
        return false;*/

#define ROUND(x) floor((x) + 0.5f)
    tMin = std::numeric_limits<float>::infinity();

    for (size_t x = position.x; x < position.x + size.x; x++) {
        for (size_t y = position.y; y < position.y + size.y; y++) {
            glm::vec3 a(POINT(x,y)), b(POINT(x+1,y)),
                      c(POINT(x,y+1)), d(POINT(x+1,y+1));
            float t, u, v;
            if (intersectTriangleWithRay(ray, a, b, d, t, u, v) && t < tMin) {
                tMin = t;
                if (ROUND(u) == 0 && ROUND(v) == 0) point = Map::Pos(x,y);
                else if (ROUND(u) == 1 && ROUND(v) == 0) point = Map::Pos(x+1,y);
                else point = Map::Pos(x+1,y+1);
            }
            if (intersectTriangleWithRay(ray, d, c, a, t, u, v) && t < tMin) {
                tMin = t;
                if (ROUND(u) == 0 && ROUND(v) == 0) point = Map::Pos(x+1,y+1);
                else if (ROUND(u) == 1 && ROUND(v) == 0) point = Map::Pos(x,y+1);
                else point = Map::Pos(x,y);
            }
        }
    }

    return tMin != std::numeric_limits<float>::infinity();
#undef ROUND
}

glm::vec3 TerrainPatch::color(size_t height) const {
    float t = (float)height / map.getMaxHeight(); 
    float b = 0.3 + t / 3;
    return glm::vec3(b, b, b);
}

TerrainMesh::TerrainMesh(const Map &map, const Map::Pos &patchSize)
    : map(map) {
    assert(map.getSizeX() % patchSize.x == 0);
    assert(map.getSizeY() % patchSize.y == 0);

    /*for (size_t x = 0; x < map.getSizeX() / patchSize.x - 1; x++) {
        for (size_t y = 0; y < map.getSizeY() / patchSize.y - 1; y++) {
            std::cout << x*patchSize.x << "," << y*patchSize.y << std::endl;
            patches.push_back(new TerrainPatch(map, Map::Pos(x*patchSize.x,y*patchSize.y),
                                               patchSize));
        }
    }*/

    patches.push_back(new TerrainPatch(map, Map::Pos(0, 0), map.getSize()-Map::Pos(1,1)));
}

void TerrainMesh::update() {
    for (auto patch : patches)
        patch->update();
}

void TerrainMesh::draw() {
    for (auto patch : patches)
        patch->draw();
}

bool TerrainMesh::intersectWithRay(const Ray &ray, Map::Pos &point, float &tMin) const {
    tMin = std::numeric_limits<float>::infinity();

    for (auto patch : patches) {
        Map::Pos p;
        float t;
        if (patch->intersectWithRay(ray, p, t) && t < tMin) {
            tMin = t;
            point = p;
        }
    }

    return tMin != std::numeric_limits<float>::infinity();
}

void TerrainMesh::init() {
    patches.push_back(
        new TerrainPatch(map, Map::Pos(0, 0), map.getSize()));
}

#undef POINT
