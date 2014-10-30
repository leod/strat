#ifndef STRAT_GAME_INTERP_STATE_HH
#define STRAT_GAME_INTERP_STATE_HH

#include "common/GameSettings.hh"

struct InterpState {
    InterpState(const GameSettings &);

    void startTick();
    void update(double dt);

    // 0 <= getT() <= 1
    double getT() const; 

    bool isTickDone() const; 

private:
    const GameSettings &settings;

    double t;
};

#endif
