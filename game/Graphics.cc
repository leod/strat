#include "Graphics.hh"

#include "Math.hh"
#include "Map.hh"
#include "InterpState.hh"

#include <GL/glu.h>
#include <inline_variant_visitor/inline_variant.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>

using namespace glm;

#define checkError() printOglError(__FILE__, __LINE__)

static vec3 playerColors[4] = {
    vec3(0.0, 1.0, 1.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
};

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

static vec3 bezier(const vec3 &a, const vec3 &b,
                        float height, float t) {
    // Bezier interpolation
    vec3 m((a.x + b.x) * 0.5,
           (a.y + b.y) * 0.5,
           (a.z + b.z) * 0.5f + height);

    vec3 a_to_m(m - a);
    vec3 da(a + a_to_m * t);
    vec3 m_to_b(b - m);
    vec3 dm(m + m_to_b * t);

    vec3 da_to_dm(dm - da);
    vec3 dda(da + da_to_dm * t);

    return dda;
}

void printOglError(const char *file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "gl error in file " << file
                  << " @ line " << line << ": "
                  << gluErrorString(error) << std::endl;
    }
}

void RenderBuildingSystem::configure(entityx::EventManager &events) {
    events.subscribe<BuildingCreated>(*this);
}

void RenderBuildingSystem::render(entityx::EntityManager &entities) {
    glDisable(GL_CULL_FACE);

    GameObject::Handle gameObject;
    Building::Handle building;
    for (entityx::Entity entity :
         entities.entities_with_components(gameObject, building)) {
        vec3 position(building->getPosition()),
             size(building->getTypeInfo().size);

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glScalef(size.x, size.y, size.z);

        assert(gameObject->getOwner() > 0 && gameObject->getOwner()-1 < 4);
        vec3 color(playerColors[gameObject->getOwner()-1]); //TODO
        glBegin(GL_QUADS);
        glColor4f(color.x, color.y, color.z, 1.0f);
        drawCube();
        glEnd();

        glPopMatrix();

        if (auto mode = boost::get<Input::BuildingSelectedMode>(&input.getMode())) {
            if (mode->isSelected(entity)) {
                glPushMatrix();
                vec3 center = position + size / 2.0f;
                float radius = sqrt((size.x * size.x) + (size.y * size.y));

                glTranslatef(center.x, center.y, center.z);

                glBegin(GL_LINE_LOOP);
                glColor3f(0.0f, 0.0f, 1.0f);

                for (float i = 0.0f; i < 2*M_PI; i += 2*M_PI / 360) {
                    glVertex3f(cos(i) * radius, sin(i) * radius, 0.0f);
                }

                glEnd();

                glPopMatrix();
            }
        }
    }

    glEnable(GL_CULL_FACE);
}

void RenderBuildingSystem::receive(const BuildingCreated &event) {
}

void RenderFlyingResourceSystem::render(entityx::EntityManager &entities) {
    glDisable(GL_CULL_FACE);

    FlyingObject::Handle flyingObject;
    FlyingResource::Handle resource;
    for (auto entity :
         entities.entities_with_components(flyingObject, resource)) {
        float ta = flyingObject->getLastProgress().toFloat();
        float tb = flyingObject->getProgress().toFloat();
        float t = lerp<float>(ta, tb, interp.getT());

        vec3 p(bezier(vec3(flyingObject->fromPosition),
                           vec3(flyingObject->toPosition),
                           flyingObject->distance.toFloat() * 0.5f,
                           t));
        vec3 c(resource->color);

        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        glTranslatef(-0.5f, -0.5f, 0.0f);

        glBegin(GL_QUADS);
        glColor4f(c.x, c.y, c.z, 1.0f);
        drawCube();
        glEnd();

        glPopMatrix();
    }

    glEnable(GL_CULL_FACE);
}

void RenderRocketSystem::render(entityx::EntityManager &entities) {
    glDisable(GL_CULL_FACE);

    FlyingObject::Handle flyingObject;
    Rocket::Handle rocket;
    for (auto entity : entities.entities_with_components(flyingObject, rocket)) {
        float ta = flyingObject->getLastProgress().toFloat();
        float tb = flyingObject->getProgress().toFloat();
        float t = lerp<float>(ta, tb, interp.getT());

        vec3 p(bezier(vec3(flyingObject->fromPosition),
                           vec3(flyingObject->toPosition),
                           flyingObject->distance.toFloat() * 0.5f,
                           t));
        vec3 p2(bezier(vec3(flyingObject->fromPosition),
                            vec3(flyingObject->toPosition),
                            flyingObject->distance.toFloat() * 0.5f,
                            t + 0.01f));
        vec3 c(1.0f, 0.0f, 0.0f);

        vec3 d(normalize(p2 - p));
        vec3 x1(d);
        vec3 x2(normalize(cross(x1, vec3(0, 0, 1))));
        vec3 x3(normalize(cross(x1, x2)));

        mat4 rot(vec4(x1, 0), vec4(x2, 0), vec4(x3, 0), vec4(0, 0, 0, 1));

        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        glTranslatef(-0.5f, -0.5f, 0.0f);
        glMultMatrixf(value_ptr(rot));
        glScalef(1.0f, 0.3f, 0.3f);

        glBegin(GL_QUADS);
        glColor4f(c.x, c.y, c.z, 1.0f);
        drawCube();
        glEnd();

        glPopMatrix();
    }

    glEnable(GL_CULL_FACE);
}

void RenderTreeSystem::render(entityx::EntityManager &entities) {
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    for (auto &part : treeObj.parts) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        assert(part.material.texture);
        part.material.texture->bind();

        part.vertices->bind();
        glVertexPointer(3, GL_FLOAT, sizeof(GLfloat)*3, nullptr);

        part.texCoords->bind();
        glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat)*2, nullptr);

        part.normals->bind();
        glNormalPointer(GL_FLOAT, sizeof(GLfloat)*3, nullptr);

        //std::cout << treeObj.parts[0].vertices->getNumElements() << std::endl;

        Tree::Handle tree;
        for (auto entity : entities.entities_with_components(tree)) {
            vec3 p(tree->getPosition());

            glPushMatrix();
            glTranslatef(p.x, p.y, p.z);
            glScalef(0.35, 0.35, 0.35);
            glRotatef(90, 1, 0, 0);
            glDrawArrays(GL_TRIANGLES, 0, part.vertices->getNumElements());
            glPopMatrix();
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glEnable(GL_CULL_FACE);
}

void setupGraphics(const Config &config, const Input::View &view) {
    checkError();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f,
        config.screenWidth / static_cast<float>(config.screenHeight),
        1.0f, 5000.0f); 
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(view.position.x, view.position.y, view.position.z,
              view.target.x, view.target.y, view.target.z,
              0.0, 0.0, 1.0);

    //GLfloat mat_specular[] = { 0.25, 0.25, 0.25, 1.0 };
    GLfloat mat_specular[] = {0,0,0,0};
    GLfloat mat_shininess[] = { 100.0 };
    GLfloat mat_diffuse[] = { 0.75, 0.75, 0.75, 1 };
    GLfloat light_position[] = { 3, 3, 5, 0.0 };
    GLfloat light_diffuse[] = { 1, 1, 1, 1 };
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_diffuse);
    glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0f);

    GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_CULL_FACE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void drawCursor(const Map &map, const Input &input) {
    glDisable(GL_CULL_FACE);

    float dz = 0.01;
    float tx = input.getCursor().x;
    float ty = input.getCursor().y;
    float tz = map.point(input.getCursor()).height + dz;

    float s = 0.2f;

    match(input.getMode(),
        [&] (const Input::DefaultMode &) {
            glBegin(GL_QUADS);
            glColor4f(1.0f, 0.0f, 1.0f, 0.5f);
            glVertex3f(tx - s, ty - s, tz);
            glVertex3f(tx + s, ty - s, tz);
            glVertex3f(tx + s, ty + s, tz);
            glVertex3f(tx - s, ty + s, tz);
            glEnd();
        },

        [&] (const Input::BuildingSelectedMode &) {
            glBegin(GL_QUADS);
            glColor4f(1.0f, 0.0f, 1.0f, 0.5f);
            glVertex3f(tx - s, ty - s, tz);
            glVertex3f(tx + s, ty - s, tz);
            glVertex3f(tx + s, ty + s, tz);
            glVertex3f(tx - s, ty + s, tz);
            glEnd();
        },

        [&] (const Input::MapSelectionMode &mode) {
            Map::Pos a(mode.start), b(input.getCursor());

            glBegin(GL_LINE_STRIP);
            glColor4f(1.0, 1.0, 1.0, 0.9f);
            for (size_t x = std::min(a.x, b.x); x <= std::max(a.x, b.x); x++) {
                glVertex3f(x, a.y, map.point(x, a.y).height + dz);
            }
            glEnd();
            glBegin(GL_LINE_STRIP);
            for (size_t x = std::min(a.x, b.x); x <= std::max(a.x, b.x); x++) {
                glVertex3f(x, b.y, map.point(x, b.y).height + dz);
            }
            glEnd();
            glBegin(GL_LINE_STRIP);
            for (size_t y = std::min(a.y, b.y); y <= std::max(a.y, b.y); y++) {
                glVertex3f(a.x, y, map.point(a.x, y).height + dz);
            }
            glEnd();
            glBegin(GL_LINE_STRIP);
            for (size_t y = std::min(a.y, b.y); y <= std::max(a.y, b.y); y++) {
                glVertex3f(b.x, y, map.point(b.x, y).height + dz);
            }
            glEnd();
        });

    glEnable(GL_CULL_FACE);
}
