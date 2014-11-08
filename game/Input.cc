#include "Input.hh"

#include "Sim.hh"
#include "Map.hh"
#include "Client.hh"
#include "Terrain.hh"

#define GLM_FORCE_RADIANS
#include <GL/glu.h>
#include <glm/gtx/rotate_vector.hpp>
#include <inline_variant_visitor/inline_variant.hpp>

#define DOUBLE_CLICK_S 0.4f

using namespace glm;

// GLFW does not allow setting a context for callbacks,
// so here we use a global variable for the current input instance
static Input *g_input = NULL;

Input::View::View()
    : angle(0),
      distance(40.0f) {
}

// Calculates a ray going from the camera position in the direction the mouse is pointing at
static Ray calculateViewRay(double mx, double my, const Input::View &view) {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];

    dvec3 nearP;
    dvec3 farP;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    gluUnProject(mx, viewport[3] - my, 0.0, modelview, projection, viewport,
                 &nearP.x, &nearP.y, &nearP.z);
    gluUnProject(mx, viewport[3] - my, 0.1, modelview, projection, viewport,
                 &farP.x, &farP.y, &farP.z);

    return Ray(view.position, vec3(normalize(farP - nearP)));
}

Input::Input(GLFWwindow *window, Client &client,
             entityx::EventManager &events, const TerrainMesh &terrain)
    : window(window), client(client), sim(client.getSim()),
      terrain(terrain), map(sim.getState().getMap()),
      mode(Input::DefaultMode()),
      scrollSpeed(5.0f) {
    view.target.x = map.getSizeX() / 2;
    view.target.y = map.getSizeY() / 2;

    assert(!g_input);
    g_input = this;

    setCallbacks(window);

    events.subscribe<entityx::EntityDestroyedEvent>(*this);
}

Input::~Input() {
    assert(g_input == this);
    g_input = NULL;
}

const Input::View &Input::getView() const {
    return view;
}

const Input::Mode &Input::getMode() const {
    return mode;
}

const Map::Pos &Input::getCursor() const {
    return cursor;
}

void Input::update(double dt) {
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    mouseRay = calculateViewRay(mx, my, view);

    // What grid point is the mouse pointing at?
    {
        float mapT;
        Map::Pos p;
        if (terrain.intersectWithRay(mouseRay, p, mapT)) {
            assert(map.isPoint(p));
            cursor = p;
        }
    }

    scrollView(dt);
}

void Input::receive(const entityx::EntityDestroyedEvent &event) {
    if (auto buildSel = boost::get<BuildingSelectedMode>(&mode)) {
        if (buildSel->isSelected(event.entity)) {
            if (buildSel->entities.size() == 1) {
                mode = DefaultMode();
            } else {
                mode = buildSel->remove(event.entity);
            }
        }
    }
}

void Input::scrollView(double dt) {
    vec2 mapDirection(view.target - view.position);
    vec2 orthDirection(cross(vec3(0, 0, 1), vec3(mapDirection, 0)));

    float moveDelta = scrollSpeed * dt;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
        && view.target.y < map.getSizeY()) {
        tryScroll(mapDirection * moveDelta);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS
        && view.target.x > 0) {
        tryScroll(orthDirection * moveDelta);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS
        && view.target.y > 0) {
        tryScroll(-mapDirection * moveDelta);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS
        && view.target.x < map.getSizeX()) {
        tryScroll(-orthDirection * moveDelta);
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS
        && view.distance > 3.0f) {
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

    view.position.x = view.target.x;
    view.position.y = view.target.y - 10.0f;
    view.position.z = view.distance;

    if (view.position.y > 0)
        view.position.z += map.point(view.position.x, view.position.y).height;
    else
        view.position.z += map.point(view.target.x, view.target.y).height;

    vec3 origin_camera(0, -10.0f, view.distance);
    view.position = view.target + rotateZ(origin_camera, view.angle);
}

void Input::tryScroll(const vec2 &delta) {
    view.target += vec3(delta, 0);

    if (view.target.x < 0) view.target.x = 0;
    if (view.target.x >= map.getSizeX()) view.target.x = map.getSizeX() - 1;
    if (view.target.y < 0) view.target.y = 0;
    if (view.target.y >= map.getSizeY()) view.target.y = map.getSizeY() - 1;

    view.target.z = map.point(view.target.x, view.target.y).height;
}

void Input::build(BuildingType type) {
    Order order(Order::BUILD);
    order.build.x = static_cast<uint16_t>(cursor.x);
    order.build.y = static_cast<uint16_t>(cursor.y);
    order.build.type = type;
    client.order(order);
}

entityx::Entity Input::pickEntity() {
    entityx::Entity minEntity;
    float minD = 0.0f;

    GameObject::Handle gameObject;
    Building::Handle building;
    for (auto entity : sim.getEntities().entities_with_components(gameObject, building)) {
        AABB aabb(vec3(building->getPosition()),
                  vec3(building->getPosition() + building->getTypeInfo().size));
        float d;

        if (aabb.intersectWithRay(mouseRay, 1.0f, 5000.0f, &d)
            && (!minEntity || d < minD)) {
            minEntity = entity;
        }
    }

    return minEntity;
}

void Input::setCallbacks(GLFWwindow *window) {
    glfwSetMouseButtonCallback(window, Input::onMouseButton);
    glfwSetKeyCallback(window, Input::onKey);
}

void Input::onMouseButton(GLFWwindow *window,
                          int button, int action, int mods) {
    if (Input *self = g_input) {
        match(self->mode,
            [&](const Input::DefaultMode &) {
                if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1) {
                    // Click on an entity?
                    if (entityx::Entity entity = self->pickEntity()) {
                        // If it's a building, switch mode
                        if (entity.has_component<Building>()) {
                            self->mode = Input::BuildingSelectedMode(entity, glfwGetTime());
                        }
                    } else { // Click on the map
                        self->mode = Input::MapSelectionMode(self->cursor);
                    }
                }
            },

            [&](const Input::BuildingSelectedMode &mode) {
                if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1) {
                    // Click on an entity?
                    if (entityx::Entity entity = self->pickEntity()) {
                        if (entity.has_component<Building>()) {
                            std::vector<entityx::Entity> selection(1, entity);

                            entityx::Entity lastEntity(mode.entities.back());

                            // Double click selection
                            if (lastEntity
                                && lastEntity == entity
                                && glfwGetTime() - mode.lastSelectionTime <= DOUBLE_CLICK_S) {
                                Building::Handle lastBuilding(lastEntity.component<Building>());
                                GameObject::Handle lastObject(lastEntity.component<GameObject>());

                                // Find all game objects of the same type and owner
                                GameObject::Handle gameObject;
                                Building::Handle building;
                                for (auto entity :
                                        self->sim.getEntities().entities_with_components(gameObject, building)) {
                                    if (building->getType() == lastBuilding->getType()
                                        && lastObject->getOwner() == gameObject->getOwner()
                                        && lastObject->getOwner() == self->client.getPlayerId()
                                        && entity != lastEntity)
                                        selection.push_back(entity);
                                }
                            }

                            if (mods & GLFW_MOD_CONTROL) { 
                                // Add to current selection
                                self->mode = mode.add(selection, glfwGetTime());
                            } else {
                                // New selection
                                self->mode = Input::BuildingSelectedMode(selection, glfwGetTime());
                            }
                        }
                    } else { // Click on the map
                        self->mode = Input::DefaultMode();
                    }
                }

                if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_2) {
                    for (auto entity : mode.entities) {
                        if (entity.component<Building>()->getType() != BUILDING_TOWER)
                            continue;

                        Order order(Order::ATTACK);
                        order.attack.x = self->cursor.x;
                        order.attack.y = self->cursor.y;
                        order.attack.objectId = entity.component<GameObject>()->getId();
                        self->client.order(order);
                    }
                }
            },

            [&](const Input::MapSelectionMode &mode) {
                if (action == GLFW_RELEASE
                    && button == GLFW_MOUSE_BUTTON_1) {
                    Map::Pos a(self->cursor), b(mode.start);

                    Order order(Order::RAISE_MAP);
                    order.raiseMap.x = std::min(a.x, b.x);
                    order.raiseMap.y = std::min(a.y, b.y);
                    order.raiseMap.w = std::max(a.x, b.x) - std::min(a.x, b.x);
                    order.raiseMap.h = std::max(a.y, b.y) - std::min(a.y, b.y);
                    self->client.order(order);

                    self->mode = Input::DefaultMode();
                } else if (action == GLFW_PRESS
                           && button == GLFW_MOUSE_BUTTON_2) {
                    self->mode = Input::DefaultMode();
                }
            });
    }
}

void Input::onKey(GLFWwindow *window, int key, int, int action, int mods) {
    if (Input *self = g_input) {
        match(self->mode,
            [&](const Input::DefaultMode &) {
                if (action == GLFW_PRESS) {
                    if (key == GLFW_KEY_B)
                        self->build(BUILDING_MINER);
                    else if (key == GLFW_KEY_N)
                        self->build(BUILDING_STORE);
                    else if (key == GLFW_KEY_M)
                        self->build(BUILDING_TOWER);
                }
            },

            [&](const Input::BuildingSelectedMode &) {
            },

            [&](const Input::MapSelectionMode &) {
            });
    }
}
