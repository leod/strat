#ifndef STRAT_GAME_INPUT_HH
#define STRAT_GAME_INPUT_HH

#include "Math.hh"
#include "Terrain.hh"

#include <GLFW/glfw3.h>

struct Client;
struct Sim;
struct Map;

struct View {
    // Camera information
    glm::vec3 position;
    glm::vec3 target;
    float angle;
    float distance;

    // Map tile the mouse is currently over
    Map::Pos cursor;
    size_t cursorHeight;

    // Map selection rectangle
    bool hasMapRectangle;
    Map::Pos mapRectangleStart;

    View();
};

// Handles user input, including:
// - scrolling, rotating and zooming the camera,
// - finding the currently selected grid point on the map,
// - selecting buildings,
// - selecting a rectangle part of the map,
// - sending orders such as BUILD and ATTACK.
//
// All the resulting information that is relevant for drawing
// is stored in a View.
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
