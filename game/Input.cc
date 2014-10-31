#include "Input.hh"

#include "Sim.hh"
#include "Map.hh"
#include "Client.hh"

#include <GL/glu.h>

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

Input::Input(GLFWwindow *window, Client &client)
    : window(window), client(client),
      sim(client.getSim()), map(sim.getState().getMap()),
      scrollSpeed(0.3f), wasPressB(false), wasPressN(false) {
    view.target.x = 64;
    view.target.y = 64;
    view.distance = 40.0f;
}

const View &Input::getView() const {
    return view;
}

void Input::update() {
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    Ray ray = calculateViewRay(mx, my, view);

    //std::cout << ray.direction.x << "," << ray.direction.y << "," << ray.direction.z << std::endl;

    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(view.position.x, view.position.y, view.position.z);
    glVertex3f(ray.direction.x, ray.direction.y, ray.direction.z);

    /*glVertex3f(view.target.x, view.target.y, view.target.z);
    glVertex3f(ray.direction.x, ray.direction.y, ray.direction.z);*/
    glEnd();

    GameObject::Handle gameObject;
    Building::Handle building;
    for (auto entity : sim.getEntities().entities_with_components(gameObject, building)) {
        AABB aabb(glm::vec3(building->getPosition()),
                  glm::vec3(building->getPosition() + building->getTypeInfo().size));

        if (aabb.intersectWithRay(ray)) {
            std::cout << "HIT " << gameObject->getId() << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
        && view.target.y < map.getSizeY()) {
        view.target.y = std::min(view.target.y + scrollSpeed, map.getSizeY()-1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS
        && view.target.x > 0) {
        view.target.x = std::max(view.target.x - scrollSpeed, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS
        && view.target.y > 0) {
        view.target.y = std::max(view.target.y - scrollSpeed, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS
        && view.target.x < map.getSizeX()) {
        view.target.x = std::min(view.target.x + scrollSpeed, map.getSizeX()-1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS && view.distance > 3.0f) {
        view.distance -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
        view.distance += 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (!wasPressB) {
            wasPressB = true;

            Order order(Order::BUILD);
            order.build.x = static_cast<uint16_t>(view.target.x);
            order.build.y = static_cast<uint16_t>(view.target.y);
            order.build.type = BUILDING_MINER;

            client.order(order);
        }
    } else wasPressB = false;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        if (!wasPressN) {
            wasPressN = true;

            Order order(Order::BUILD);
            order.build.x = static_cast<uint16_t>(view.target.x);
            order.build.y = static_cast<uint16_t>(view.target.y);
            order.build.type = BUILDING_STORE;

            client.order(order);
        }
    } else wasPressN = false;

    view.target.z = map.point(view.target.x, view.target.y).height;

    view.position.x = view.target.x;
    view.position.y = view.target.y - 15.0f;
    view.position.z = view.target.z + view.distance;
}
