#ifndef STRAT_COMMON_MESSAGE_HH
#define STRAT_COMMON_MESSAGE_HH

#include "Order.hh"
#include "GameSettings.hh"

#include <enet/enet.h>

#include <vector>
#include <string>

struct BitStreamReader;
struct BitStreamWriter;

struct Message {
    enum Type {
        UNDEFINED,

        // Messages sent by client
        CLIENT_CONNECT,
        CLIENT_ORDER,
        CLIENT_TICK_DONE,

        // Messages sent by server
        SERVER_CONNECT,
        SERVER_TICK,
        SERVER_START,
    };

    Type type;

    Message(Type);
    ~Message();

    struct ClientConnect {
        std::string name;
    };

    struct ServerTick {
        std::vector<Order> orders;
    };

    struct ServerStart {
        GameSettings settings;
    };

    union {
        ClientConnect client_connect;

        struct {
            Order order;
        } client_order;

        struct {
            PlayerId yourPlayerId;
        } server_connect;

        ServerStart server_start;
        ServerTick server_tick;
    };

    ENetPacket *toPacket() const;
};

// NOTE: Assumes message type has already been read
void read(BitStreamReader &, Message &);

void write(BitStreamWriter &, const Message &);

#endif
