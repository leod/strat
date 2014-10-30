#include "Input.hh"

#include "Sim.hh"
#include "Map.hh"
#include "Client.hh"

Input::Input(GLFWwindow *window, Client &client)
    : window(window), client(client),
      map(client.getSim().getState().getMap()),
      scrollSpeed(0.3f), wasPressB(false), wasPressN(false) {
    view.targetX = 64;
    view.targetY = 64;
    view.distance = 40.0f;
}

const View &Input::getView() const {
    return view;
}

void Input::update() {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
        && view.targetY < map.getSizeY()) {
        view.targetY = std::min(view.targetY + scrollSpeed, map.getSizeY()-1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS
        && view.targetX > 0) {
        view.targetX = std::max(view.targetX - scrollSpeed, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS
        && view.targetY > 0) {
        view.targetY = std::max(view.targetY - scrollSpeed, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS
        && view.targetX < map.getSizeX()) {
        view.targetX = std::min(view.targetX + scrollSpeed, map.getSizeX()-1.0f);
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
            order.build.x = static_cast<uint16_t>(view.targetX);
            order.build.y = static_cast<uint16_t>(view.targetY);
            order.build.type = BUILDING_MINER;

            client.order(order);
        }
    } else wasPressB = false;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        if (!wasPressN) {
            wasPressN = true;

            Order order(Order::BUILD);
            order.build.x = static_cast<uint16_t>(view.targetX);
            order.build.y = static_cast<uint16_t>(view.targetY);
            order.build.type = BUILDING_STORE;

            client.order(order);
        }
    } else wasPressN = false;

    view.height = map.point(view.targetX, view.targetY).height;
}
