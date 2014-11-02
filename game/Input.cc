#include "Input.hh"

#include "Sim.hh"
#include "Map.hh"
#include "Client.hh"

#define GLM_FORCE_RADIANS
#include <GL/glu.h>
#include <glm/gtx/rotate_vector.hpp>

View::View()
    : angle(0),
      distance(40.0f),
      cursorHeight(0),
      hasMapRectangle(false) {
}

// Calculates a ray going from the camera position in the direction the mouse is pointing
static Ray calculateViewRay(double mx, double my, const View &view) {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];

    glm::dvec3 nearP;
    glm::dvec3 farP;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    gluUnProject(mx, viewport[3] - my, 0.0, modelview, projection, viewport,
                 &nearP.x, &nearP.y, &nearP.z);
    gluUnProject(mx, viewport[3] - my, 0.1, modelview, projection, viewport,
                 &farP.x, &farP.y, &farP.z);

    return Ray(view.position, glm::vec3(glm::normalize(farP - nearP)));
}

Input::Input(GLFWwindow *window, Client &client, const TerrainMesh &terrain)
    : window(window), client(client), sim(client.getSim()),
      terrain(terrain), map(sim.getState().getMap()),
      scrollSpeed(5.0f), wasPressB(false), wasPressN(false) {
    view.target.x = map.getSizeX() / 2;
    view.target.y = map.getSizeY() / 2;
}

const View &Input::getView() const {
    return view;
}

void Input::update(double dt) {
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    Ray ray = calculateViewRay(mx, my, view);

    GameObject::Handle gameObject;
    Building::Handle building;
    for (auto entity : sim.getEntities().entities_with_components(gameObject, building)) {
        AABB aabb(glm::vec3(building->getPosition()),
                  glm::vec3(building->getPosition() + building->getTypeInfo().size));

        if (aabb.intersectWithRay(ray, 1.0f, 5000.0f)) {
            //std::cout << "HIT " << gameObject->getId() << std::endl;
        }
    }


    float mapT;
    Map::Pos cursor;
    if (terrain.intersectWithRay(ray, cursor, mapT)) {
        assert(map.isPoint(cursor));

        view.cursor = cursor;
        view.cursorHeight = map.point(cursor).height;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        if (!view.hasMapRectangle) {
            view.mapRectangleStart = view.cursor;
            view.hasMapRectangle = true;
        }
    } else {
        if (view.hasMapRectangle) {
            Map::Pos a(view.cursor), b(view.mapRectangleStart);

            Order order(Order::RAISE_MAP);
            order.raiseMap.x = std::min(a.x, b.x);
            order.raiseMap.y = std::min(a.y, b.y);
            order.raiseMap.w = std::max(a.x, b.x) - std::min(a.x, b.x);
            order.raiseMap.h = std::max(a.y, b.y) - std::min(a.y, b.y);
            client.order(order);
        }

        view.hasMapRectangle = false;
    }

    glm::vec2 mapDirection(view.target - view.position);

    float moveDelta = scrollSpeed * dt;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && view.target.y < map.getSizeY()) {
        glm::vec2 scroll(mapDirection * moveDelta);
        tryScroll(scroll);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && view.target.x > 0) {
        glm::vec2 scroll(glm::cross(glm::vec3(0, 0, 1), glm::vec3(mapDirection, 0)) * moveDelta);
        tryScroll(scroll);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && view.target.y > 0) {
        glm::vec2 scroll(-mapDirection * moveDelta);
        tryScroll(scroll);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && view.target.x < map.getSizeX()) {
        glm::vec2 scroll(-glm::cross(glm::vec3(0, 0, 1), glm::vec3(mapDirection, 0)) * moveDelta);
        tryScroll(scroll);
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS && view.distance > 0.0f) {
        view.distance -= dt * 35.0f;
        if (view.distance < 3.0f)
            view.distance = 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
        view.distance += dt * 35.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        view.angle += dt * 2.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        view.angle -= dt * 2.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (!wasPressB) {
            wasPressB = true;

            Order order(Order::BUILD);
            order.build.x = static_cast<uint16_t>(view.cursor.x);
            order.build.y = static_cast<uint16_t>(view.cursor.y);
            order.build.type = BUILDING_MINER;
            client.order(order);
        }
    } else wasPressB = false;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        if (!wasPressN) {
            wasPressN = true;

            Order order(Order::BUILD);
            order.build.x = static_cast<uint16_t>(view.cursor.x);
            order.build.y = static_cast<uint16_t>(view.cursor.y);
            order.build.type = BUILDING_STORE;
            client.order(order);
        }
    } else wasPressN = false;

    view.position.x = view.target.x;
    view.position.y = view.target.y - 10.0f;
    view.position.z = view.target.z + view.distance;

    glm::vec3 origin_camera(0, -10.0f, view.distance);
    view.position = view.target + glm::rotateZ(origin_camera, view.angle);
}

void Input::tryScroll(const glm::vec2 &delta) {
    view.target += glm::vec3(delta, 0);

    if (view.target.x < 0) view.target.x = 0;
    if (view.target.x >= map.getSizeX()) view.target.x = map.getSizeX() - 1;
    if (view.target.y < 0) view.target.y = 0;
    if (view.target.y >= map.getSizeY()) view.target.y = map.getSizeY() - 1;

    view.target.z = map.point(view.target.x, view.target.y).height;
}
