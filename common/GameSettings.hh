#ifndef STRAT_COMMON_GAME_SETTINGS_HH
#define STRAT_COMMON_GAME_SETTINGS_HH

#include "Defs.hh"

#include <vector>
#include <string>

struct BitStreamReader;
struct BitStreamWriter;

struct PlayerInfo {
    PlayerId id;
    std::string name;
    uint32_t team;
    uint8_t color;
};

void read(BitStreamReader &, PlayerInfo &);
void write(BitStreamWriter &, PlayerInfo &);

struct GameSettings {
    std::vector<PlayerInfo> players;
    uint32_t randomSeed;
    uint32_t mapW, mapH; 
    uint32_t heightLimit;
    uint32_t tickLengthMs;
};

void read(BitStreamReader &, GameSettings &);
void write(BitStreamWriter &, const GameSettings &);

#endif
