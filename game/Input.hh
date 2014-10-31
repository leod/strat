#ifndef STRAT_GAME_INPUT_HH
#define STRAT_GAME_INPUT_HH

#include "Math.hh"

#include <GLFW/glfw3.h>

struct Client;
struct Sim;
struct Map;

struct View {
    glm::vec3 position;
    glm::vec3 target;

    float angle;

    float distance;
};

struct Input {
    Input(GLFWwindow *, Client &);

    const View &getView() const;

    void update(double dt); 

private:
    GLFWwindow *window;

    Client &client;
    Sim &sim;
    const Map &map;

    float scrollSpeed;
    bool wasPressB, wasPressN;

    View view;

    void tryScroll(const glm::vec2 &delta);
};

#endif
