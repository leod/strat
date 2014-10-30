#ifndef STRAT_GAME_INPUT_HH
#define STRAT_GAME_INPUT_HH

#include "Graphics.hh"

#include <GLFW/glfw3.h>

struct Client;
struct Map;

struct Input {
    Input(GLFWwindow *, Client &);

    const View &getView() const;

    void update(); 

private:
    GLFWwindow *window;

    Client &client;
    const Map &map;

    float scrollSpeed;
    bool wasPressB, wasPressN;

    View view;
};

#endif
