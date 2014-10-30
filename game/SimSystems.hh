#ifndef STRAT_GAME_SIM_SYSTEMS_HH
#define STRAT_GAME_SIM_SYSTEMS_HH

struct SimState;

struct MinerBuildingSystem {
    void tick(SimState &);    
};

struct ResourceTransferSystem {
    void tick(SimState &);    
};

#endif
