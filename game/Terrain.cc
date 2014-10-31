#include "Terrain.hh"

#include <GL/glu.h>

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
    void draw();

private:
    const Map &map;
    Map::Pos position, size;

    glm::vec3 color(size_t height) const; 

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    GLuint vertexBuffer;
    GLuint indexBuffer;
};

TerrainPatch::TerrainPatch(const Map &map,
                           const Map::Pos &position,
                           const Map::Pos &size)
    : map(map), position(position), size(size) {
    init();
}

void TerrainPatch::init() {
#define POINT(x,y) glm::vec3(x, y, map.point(x, y).height)
    // Generate one vertex per map point
    /*for (size_t x = position.x;
         x < position.x + size.x;
         x++) {
        for (size_t y = position.y;
             y < position.y + size.y;
             y++) {
            glm::vec3 n;
            if (x > 0 && y > 0) {
                n += glm::cross(POINT(x-1,y) - POINT(x,y),
                                POINT(x-1,y-1) - POINT(x,y));
                n += glm::cross(POINT(x-1,y-1) - POINT(x,y),
                                POINT(x,y-1) - POINT(x,y));
            }
            if (x < map.getSizeX() - 1 && y > 0) {
                n += glm::cross(POINT(x,y-1) - POINT(x,y),
                                POINT(x+1,y) - POINT(x,y));
                n += glm::cross(POINT(x,y-1) - POINT(x+1,y),
                                POINT(x+1,y-1) - POINT(x+1,y));
            }
            if (x < map.getSizeX() - 1 && y < map.getSizeY() - 1) {
                n += glm::cross(POINT(x+1,y) - POINT(x,y),
                                POINT(x+1,y+1) - POINT(x,y));
                n += glm::cross(POINT(x+1,y+1) - POINT(x,y),
                                POINT(x+1,y) - POINT(x,y));
            }
            if (x > 0 && y < map.getSizeY() - 1) {
                n += glm::cross(POINT(x,y+1) - POINT(x,y),
                                POINT(x-1,y) - POINT(x,y));
                n += glm::cross(POINT(x,y+1) - POINT(x-1,y),
                                POINT(x-1,y+1) - POINT(x-1,y));
            }
            n = glm::normalize(n);

            vertices.push_back(Vertex(POINT(x,y), color(POINT(x,y).z), n));
        }
    }

    // Generate one quad for almost each map point
    for (size_t x = position.x;
         x < position.x + size.x - 1;
         x++) {
        for (size_t y = position.y;
             y < position.y + size.y - 1;
             y++) {
            indices.push_back(map.getSizeY() * x + y);
            indices.push_back(map.getSizeY() * (x+1) + y);
            indices.push_back(map.getSizeY() * (x+1) + y+1);
            indices.push_back(map.getSizeY() * x + y+1);
        }
    }

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(GLuint) * indices.size(), &indices[0],
        GL_STATIC_DRAW);*/

    for (size_t x = 0; x < map.getSizeX() - 1; x++) {
        for (size_t y = 0; y < map.getSizeY() - 1; y++) {
            glm::vec3 a(POINT(x,y));
            glm::vec3 b(POINT(x+1,y));
            glm::vec3 c(POINT(x,y+1));
            glm::vec3 d(POINT(x+1,y+1));

            glm::vec3 n1(glm::normalize(glm::cross(a - d, b - d)));
            glm::vec3 n2(glm::normalize(glm::cross(c - d, a - d)));

            Vertex va1(d, color(d.z), n1);
            Vertex vb1(b, color(b.z), n1);
            Vertex vc1(a, color(a.z), n1);

            Vertex va2(a, color(a.z), n2);
            Vertex vb2(c, color(c.z), n2);
            Vertex vc2(d, color(d.z), n2);

            vertices.push_back(va1);
            vertices.push_back(vb1);
            vertices.push_back(vc1);

            vertices.push_back(va2);
            vertices.push_back(vb2);
            vertices.push_back(vc2);
        }
    }
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
        &vertices[0], GL_STATIC_DRAW);

#undef POINT
}

void TerrainPatch::draw() {
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer); 
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    glEnableClientState(GL_VERTEX_ARRAY); // oldschool yo
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vertex),
        reinterpret_cast<const void *>(offsetof(Vertex, position)));
    glColorPointer(3, GL_FLOAT, sizeof(Vertex),
        reinterpret_cast<const void *>(offsetof(Vertex, color)));
    glNormalPointer(GL_FLOAT, sizeof(Vertex),
        reinterpret_cast<const void *>(offsetof(Vertex, normal)));

    /*glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_INT,
                   NULL);*/
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

glm::vec3 TerrainPatch::color(size_t height) const {
    float t = (float)height / map.getMaxHeight(); 
    float b = 0.3 + t / 3;
    return glm::vec3(b, b, b);
}

TerrainMesh::TerrainMesh(const Map &map)
    : map(map) {
    init();
}

void TerrainMesh::init() {
    patches.push_back(
        new TerrainPatch(map, Map::Pos(0, 0), map.getSize()));
}

void TerrainMesh::draw() {
    for (auto patch : patches)
        patch->draw();
}

