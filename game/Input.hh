#ifndef STRAT_GAME_INPUT_HH
#define STRAT_GAME_INPUT_HH

#include "Math.hh"
#include "Map.hh"
#include "common/Defs.hh"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <entityx/entityx.h>
#include <boost/variant.hpp>

#include <algorithm>

struct Config;
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
struct Input : entityx::Receiver<Input> {
    // These are the modes the input can be in:
    //
    //             build shortcuts
    // DefaultMode --------------------> DefaultMode
    //             order build
    //             
    //             left click building
    // DefaultMode --------------------> BuildingSelectedMode
    //
    //             left click map
    // DefaultMode --------------------> MapSelectionMode
    struct DefaultMode {
    };

    //                      left click building
    // BuildingSelectedMode -----------> BuildingSelectedMode
    //
    //                    right click map
    // BuildingSelectMode -------------> BuildingSelectedMode
    //                    order send rocket
    //
    //                      left click map
    // BuildingSelectedMode -----------> DefaultMode
    struct BuildingSelectedMode {
        std::vector<entityx::Entity> entities;
        double lastSelectionTime;

        BuildingSelectedMode(entityx::Entity entity, double time);
        BuildingSelectedMode(const std::vector<entityx::Entity> &entities,
                             double time);

        BuildingSelectedMode add(entityx::Entity entity, double time) const;
        BuildingSelectedMode add(const std::vector<entityx::Entity> es,
                                 double time) const;

        bool isSelected(entityx::Entity e) const;

        BuildingSelectedMode remove(entityx::Entity e) const;
    };

    //                  release left mouse button
    // MapSelectionMode ---------------> DefaultMode
    //                  order raising map
    //
    //                  right click
    // MapSelectionMode ---------------> DefaultMode
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
        glm::vec3 position, target;
        float angle, distance;

        glm::mat4 projectionMat, cameraMat;

        View();
    };

    Input(const Config &, GLFWwindow *, Client &, entityx::EventManager &,
          const TerrainMesh &);
    ~Input();

    const View &getView() const;
    const Mode &getMode() const;
    const Map::Pos &getCursor() const;

    void update(double dt); 

    void receive(const entityx::EntityDestroyedEvent &);

private:
    const Config &config;
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
