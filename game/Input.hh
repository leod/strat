#ifndef STRAT_GAME_INPUT_HH
#define STRAT_GAME_INPUT_HH

#include "Math.hh"
#include "Map.hh"
#include "common/Defs.hh"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <entityx/entityx.h>
#include <boost/variant.hpp>

struct Client;
struct Sim;
struct TerrainMesh;

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
    // These are the modes the input can be in:
    //
    //             left click building
    // DefaultMode --------------------> BuildingSelectedMode
    //
    //             build shortcuts
    // DefaultMode --------------------> DefaultMode
    //             order build
    //             
    //             left click map
    // DefaultMode --------------------> MapSelectionMode
    //
    //                  release left mouse button
    // MapSelectionMode ---------------> DefaultMode
    //                  order raising map
    //
    //                  right click
    // MapSelectionMode ---------------> DefaultMode
    //
    //                      left click building
    // BuildignSelectedMode -----------> BuildingSelectedMode
    //
    //                    right click map
    // BuildingSelectMode -------------> BuildingSelectedMode
    //                    order send rocket
    struct DefaultMode {
    };

    struct BuildingSelectedMode {
        entityx::Entity entity;

        BuildingSelectedMode(entityx::Entity entity)
            : entity(entity) {
        }
    };

    struct MapSelectionMode {
        Map::Pos start;

        MapSelectionMode(Map::Pos start)
            : start(start) {
        }
    };

    typedef boost::variant<DefaultMode,
                           BuildingSelectedMode,
                           MapSelectionMode> Mode;

    // Camera information
    struct View {
        glm::vec3 position;
        glm::vec3 target;

        float angle;
        float distance;

        View();
    };

    Input(GLFWwindow *, Client &, const TerrainMesh &);
    ~Input();

    const View &getView() const;
    const Mode &getMode() const;
    const Map::Pos &getCursor() const;

    void update(double dt); 

private:
    GLFWwindow *window;

    Client &client;
    Sim &sim;
    const TerrainMesh &terrain;
    const Map &map;

    Mode mode;

    View view;
    Ray mouseRay;
    float scrollSpeed;

    // Map tile the mouse is currently over
    Map::Pos cursor;

    void scrollView(double dt);

    // Scroll as far as possible on the map using delta
    void tryScroll(const glm::vec2 &delta);

    // Send order for building a building at the cursor position
    void build(BuildingType);

    // Intersect view ray with selectable objects,
    // returns the closest hit or an invalid entity handle if there is none
    entityx::Entity pickEntity();
 
    // Callbacks for GLFW
    static void setCallbacks(GLFWwindow *);

    static void onMouseButton(GLFWwindow *, int, int, int);
    static void onKey(GLFWwindow *, int, int, int, int);

    // disallow:
    Input(const Input &);
    void operator=(const Input &);
};

#endif
