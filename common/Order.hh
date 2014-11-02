#ifndef STRAT_COMMON_ORDER_HH
#define STRAT_COMMON_ORDER_HH

#include "Defs.hh"

struct BitStreamReader;
struct BitStreamWriter;

struct Order {
    enum Type {
        UNDEFINED,
        BUILD,
        ATTACK,
        STOP,
        REMOVE,
        RAISE_MAP
    } type;

    Order(Type type = UNDEFINED)
        : type(type) {
    }

    PlayerId player;

    union {
        struct {
            BuildingType type;
            uint16_t x, y;
        } build;

        struct {
            ObjectId objectId;
            uint16_t x, y;
        } attack;

        struct {
            ObjectId objectId;
        } stop;

        struct { 
            ObjectId objectId;
        } remove;

        struct {
            uint16_t x, y, w, h;
        } raiseMap;
    };
};

void read(BitStreamReader &, Order &);
void write(BitStreamWriter &, const Order &);

#endif
