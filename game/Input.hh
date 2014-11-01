#ifndef STRAT_GAME_INPUT_HH
#define STRAT_GAME_INPUT_HH

#include "Math.hh"
#include "Terrain.hh"

#include <GLFW/glfw3.h>

struct Client;
struct Sim;
struct Map;

struct View {
    glm::vec3 position;
    glm::vec3 target;

    float angle;
    float distance;

    Map::Pos cursor;
};

struct Input {
    Input(GLFWwindow *, Client &, const TerrainMesh &);

    const View &getView() const;

    void update(double dt); 

private:
    GLFWwindow *window;

    Client &client;
    Sim &sim;
    const TerrainMesh &terrain;
    const Map &map;

    float scrollSpeed;
    bool wasPressB, wasPressN;

    View view;

    void tryScroll(const glm::vec2 &delta);
};

#endif
