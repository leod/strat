#ifndef STRAT_GAME_CLIENT_HH
#define STRAT_GAME_CLIENT_HH

#include "Sim.hh"
#include "common/Message.hh"

#include <enet/enet.h>
#include <entityx/entityx.h>

#include <string>

struct Client {
    Client(entityx::EntityX &entityx, const std::string &username);
    ~Client();

    void connect(const std::string &host, int port);

    Sim &getSim() {
        return sim;
    }

    const Sim &getSim() const {
        return sim;
    }    

    void update();

    void order(const Order &order);

private:
    entityx::EntityX &entityx;

    std::string username;

    ENetHost *client;
    ENetPeer *peer;

    GameSettings settings;
    Sim sim;

    PlayerId playerId;

    bool tickRunning;
    double tickStartTime;

    void sendMessage(const Message &);
    void handleMessage(const Message &);
};

#endif
