#include "game/InterpState.hh"

#include <iostream>

InterpState::InterpState(const GameSettings &settings)
    : settings(settings), t(0) {

}

void InterpState::startTick() {
    t = 0;
}

void InterpState::update(double dt) {
    double tickLengthS = settings.tickLengthMs / 1000.0f;

    t += dt / tickLengthS;

    if (t >= 1.0)
        t = 1.0;
}

double InterpState::getT() const {
    return t;
}

bool InterpState::isTickDone() const { 
    return t == 1.0;
}
