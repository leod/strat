#include "Graphics.hh"
#include "Config.hh"
#include "Map.hh"
#include "Client.hh"

#include <entityx/entityx.h>

#include <cstdlib>

#include <GL/glew.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>

void errorCallback(int error, const char *description) {
    std::cerr << "GLFW error: " << description << std::endl;
}

int main(int argc, char *argv[]) {
    Config config;

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed" << std::endl;
        return 1;
    }

    glfwSetErrorCallback(errorCallback);

    //glfwWindowHint(GLFW_SAMPLES, 4);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    GLFWwindow *window = glfwCreateWindow(config.screenWidth,
        config.screenHeight, "Strats", NULL, NULL);

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed" << std::endl;
        return 1;
    }

    if (!window) {
        glfwTerminate();
        return 1;
    }

    View view;
    view.targetX = 64;
    view.targetY = 64;
    view.distance = 40.0f;

    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet" << std::endl;
        return 1;
    }

    Client client("leo");
    client.connect("192.168.11.41", 1234);

    while (!client.isStarted()) {
        std::cout << "Waiting for the game to start" << std::endl;
        client.update();
    }

    Sim &sim(client.getSim());
    SimState &simState(sim.getState());
    Map &map(simState.getMap());
    TerrainMesh terrainMesh(map);

    simState.systems.add<RenderBuildingSystem>(map);
    simState.systems.configure();

    size_t frames = 0, fps = 0;
    double lastFrameTime = glfwGetTime();

    bool quit = false;
    bool wasPressB = false, wasPressN = false;
    while (!quit && !glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        client.update();

        setupGraphics(config, view);
        terrainMesh.draw();
        
        simState.systems.system<RenderBuildingSystem>()->render(simState.entities);

        drawCursor(view);

        float scrollSpeed = 0.3;
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
                order.build.type = BUILDING_STORE;

                client.order(order);
            }
        } else wasPressB = false;
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            if (!wasPressN) {
                wasPressN = true;

                Order order(Order::BUILD);
                order.build.x = static_cast<uint16_t>(view.targetX);
                order.build.y = static_cast<uint16_t>(view.targetY);
                order.build.type = BUILDING_BASE;

                client.order(order);
            }
        } else wasPressN = false;

        view.height = map.point(view.targetX, view.targetY).height;

        glfwSwapBuffers(window);
        glfwPollEvents();

        frames++;
        if (glfwGetTime() - lastFrameTime >= 1.0f) {
            fps = frames;
            frames = 0;
            lastFrameTime += 1.0f;
        }

        std::stringstream ss;
        ss << "Strats (" << fps << " FPS, "
                         << "x=" << static_cast<int>(view.targetX) << ", "
                         << "y=" << static_cast<int>(view.targetY) << ", "
                         << "z=" << static_cast<int>(view.height)
                         << ")";
        glfwSetWindowTitle(window, ss.str().c_str());
    } 

    enet_deinitialize();
    glfwTerminate();

    return 0;
}
