#ifndef STRAT_GAME_CLIENT_HH
#define STRAT_GAME_CLIENT_HH

#include "Sim.hh"
#include "InterpState.hh"
#include "common/Message.hh"

#include <enet/enet.h>
#include <entityx/entityx.h>

#include <string>

// The client connects to the specified game server,
// and then runs a game simulation with the settings given by the server.
//
// Everytime the server sends a tick to the client, it is
// executed in the local simulation.
struct Client {
    Client(const std::string &username);
    ~Client();

    void connect(const std::string &host, int port);

    Sim &getSim() {
        assert(sim != NULL);
        return *sim;
    }

    const Sim &getSim() const {
        assert(sim != NULL);
        return *sim;
    }    

    void update(double dt);

    void order(const Order &order);

    bool isStarted() const {
        return sim != NULL;
    }

    const InterpState &getInterp() const {
        return interp;
    }

    PlayerId getPlayerId() const {
        assert(playerId > 0);
        return playerId;
    }

private:
    std::string username;

    ENetHost *client;
    ENetPeer *peer;

    GameSettings settings;
    Sim *sim;

    PlayerId playerId;

    bool tickRunning;
    InterpState interp;

    bool haveQueuedTick;
    std::vector<Order> queuedOrders;

    void sendMessage(const Message &);
    void handleMessage(const Message &);
};

#endif
