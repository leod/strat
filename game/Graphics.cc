#include "Graphics.hh"

#include "Math.hh"
#include "Map.hh"

#include <GL/glu.h>

#include <vector>
#include <iostream>

#define checkError() printOglError(__FILE__, __LINE__)

static glm::vec3 playerColors[4] = {
    glm::vec3(0.0, 1.0, 1.0),
    glm::vec3(1.0, 1.0, 0.0),
    glm::vec3(0.0, 1.0, 0.0),
    glm::vec3(0.0, 0.0, 1.0)
};

void printOglError(const char *file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "gl error in file " << file
                  << " @ line " << line << ": "
                  << gluErrorString(error) << std::endl;
    }
}

TerrainMesh::TerrainMesh(const Map &map)
    : map(map) {
    init();
}

void TerrainMesh::init() {
    for (size_t x = 0; x < map.getSizeX() - 1; x++) {
        for (size_t y = 0; y < map.getSizeY() - 1; y++) {
            glm::vec3 a(x, y, map.point(x, y).height);
            glm::vec3 b(x+1, y, map.point(x+1, y).height);
            glm::vec3 c(x+1, y+1, map.point(x+1, y+1).height);
            glm::vec3 d(x, y+1, map.point(x, y+1).height);

            glm::vec3 ca(color(a.z));
            glm::vec3 cb(color(b.z));
            glm::vec3 cc(color(c.z));
            glm::vec3 cd(color(d.z));

            glm::vec3 n(glm::normalize(glm::cross(b - a, c - a)));

            Vertex va(a, ca, n);
            Vertex vb(b, cb, n);
            Vertex vc(c, cc, n);
            Vertex vd(d, cd, n);
            
            vertices.push_back(va);
            vertices.push_back(vb);
            vertices.push_back(vc);
            vertices.push_back(vd);
        }
    }
    
    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
        &vertices[0], GL_STATIC_DRAW);

    checkError();
}

void TerrainMesh::draw() {
    checkError();

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

    glDrawArrays(GL_QUADS, 0, vertices.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return;

    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 0.0);

    for (size_t x = 0; x < map.getSizeX() - 1; x++) {
        for (size_t y = 0; y < map.getSizeY() - 1; y++) {
            glm::vec3 a(x, y, map.point(x, y).height);
            glm::vec3 b(x+1, y, map.point(x+1, y).height);
            glm::vec3 c(x+1, y+1, map.point(x+1, y+1).height);
            glm::vec3 d(x, y+1, map.point(x, y+1).height);

            float dz = 0.05f;

            glVertex3f(a.x, a.y, a.z+dz);
            glVertex3f(b.x, b.y, b.z+dz);

            glVertex3f(a.x, a.y, a.z+dz);
            glVertex3f(d.x, d.y, d.z+dz);

            glVertex3f(b.x, b.y, b.z+dz);
            glVertex3f(c.x, c.y, c.z+dz);

            glVertex3f(c.x, c.y, c.z+dz);
            glVertex3f(d.x, d.y, d.z+dz);
        }
    }

    glEnd();
}

glm::vec3 TerrainMesh::color(size_t height) {
    /*if (height == 0) return glm::vec3(1, 0, 1);
    else if (height == 1) return glm::vec3(1, 0, 1);
    else if (height == 2) return glm::vec3(1, 1, 0);
    else if (height == 3) return glm::vec3(1, 1, 0);
    else if (height == 4) return glm::vec3(0, 0, 1);
    else if (height == 5) return glm::vec3(0, 0, 1);
    else if (height == 6) return glm::vec3(1, 1, 0);
    else if (height == 7) return glm::vec3(1, 1, 0);
    else if (height == 8) return glm::vec3(1, 0, 1);
    else if (height == 9) return glm::vec3(1, 0, 1);
    else if (height == 10) return glm::vec3(0, 1, 1);*/
        

    float t = (float)height / map.getMaxHeight(); 
    //assert(t >= 0 && t <= 1);
    //
    //return glm::vec3(0.5f, 0.5f, 0.5f);

    return glm::vec3(0.4f + t/3, 0.4f + t/3, 0.4f + t/3);
}

static void drawCube() {
    // front
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    // back
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, 1.0f, 1.0f);
    // right
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 0.0f);
    // left
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    // top
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, 1.0f, 1.0f);
    // bottom
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
}

void RenderBuildingSystem::configure(entityx::EventManager &events) {
    events.subscribe<BuildingCreated>(*this);
}

void RenderBuildingSystem::render(entityx::EntityManager &entities) {
    GameObject::Handle gameObject;
    Building::Handle building;
    for (entityx::Entity entity:
         entities.entities_with_components(gameObject, building)) {
        glPushMatrix();
        glTranslatef(building->getX(), building->getY(),
                     map.point(building->getX(), building->getY()).height);
        glScalef(building->getTypeInfo().sizeX - 1.0f,
                 building->getTypeInfo().sizeY - 1.0f,
                 building->getTypeInfo().sizeZ);
        assert(gameObject->getOwner() > 0 && gameObject->getOwner()-1 < 4);
        glm::vec3 color(playerColors[gameObject->getOwner()-1]);
        glBegin(GL_QUADS);
        glColor4f(color.x, color.y, color.z, 1.0f);
        drawCube();
        glEnd();
        glPopMatrix();
    }
}

void RenderBuildingSystem::receive(const BuildingCreated &event) {
}

void setupGraphics(const Config &config, const View &view) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f,
        config.screenWidth / static_cast<float>(config.screenHeight),
        1.0f, 5000.0f); 
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(view.targetX, view.targetY, view.height + view.distance,
              view.targetX, view.targetY, view.height,
              0.0, 1.0, 0.0);

    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    GLfloat light_position[] = { 1, 1, 1, 0.0 };
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_CULL_FACE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void drawCursor(const View &view) {
    float tx = static_cast<float>(static_cast<int>(view.targetX));
    float ty = static_cast<float>(static_cast<int>(view.targetY));
    float tz = view.height + 0.001;

    float s = 0.5f;

    glBegin(GL_QUADS);
    glColor4f(1.0f, 0.0f, 1.0f, 0.5f);
    glVertex3f(tx - s, ty - s, tz);
    glVertex3f(tx + s, ty - s, tz);
    glVertex3f(tx + s, ty + s, tz);
    glVertex3f(tx - s, ty + s, tz);
    glEnd();
}
