#include <enet/enet.h>

#include <algorithm>
#include <iostream>
#include <cassert>
#include <ctime>
#include <vector>

#include "common/Message.hh"
#include "common/GameSettings.hh"
#include "common/BitStream.hh"

struct ClientInfo {
    ENetPeer *peer;

    size_t ticksDone;

    PlayerInfo player;

    ClientInfo(PlayerId id, ENetPeer *peer)
        : peer(peer), ticksDone(0), player() {
        player.id = id;
    }
};

GameSettings settings;

ENetHost *server = NULL;
bool gameStarted = false;

PlayerId playerCounter = 0;
std::vector<ClientInfo *> clients;

size_t ticksStarted = 0;
std::vector<Order> nextOrders;

void sendMessage(ClientInfo *client, const Message &message) {
    assert(client && client->peer);

    ENetPacket *packet = message.toPacket();
    enet_peer_send(client->peer, 0, packet);
}

void broadcast(const Message &message) {
    for (auto client : clients)
        sendMessage(client, message);
}

void startTick() {
    assert(gameStarted);
    for (auto client : clients) {
        assert(client->ticksDone <= ticksStarted);
        assert(ticksStarted - client->ticksDone <= 2);

        std::cout << "Client at " << client->ticksDone << std::endl;
    }

    std::cout << "Starting tick " << ticksStarted + 1 << std::endl;

    Message message(Message::SERVER_TICK);
    message.server_tick.orders = nextOrders;
    broadcast(message);

    ticksStarted++;
    nextOrders.clear();

    /*for (auto client : clients)
        client->ticksDone = false;*/
}

void startGame() {
    std::cout << "All players connected; starting game" << std::endl;

    assert(!gameStarted);
    assert(settings.players.empty());

    for (auto client : clients)
        settings.players.push_back(client->player);

    Message message(Message::SERVER_START);
    message.server_start.settings = settings;
    broadcast(message);

    gameStarted = true;
}

void handleMessage(ClientInfo *client, const Message &message) {
    switch (message.type) {
    case Message::CLIENT_CONNECT: {
        client->player.name = message.client_connect.name;

        Message message(Message::SERVER_CONNECT);
        message.server_connect.yourPlayerId = client->player.id;
        sendMessage(client, message);
        return;
    }
    case Message::CLIENT_ORDER: {
        if (!gameStarted) {
            std::cout << "Ignoring order from player " << client->player.id << std::endl;
            return;
        }

        nextOrders.push_back(message.client_order.order);
        nextOrders.back().player = client->player.id;

        return;
    }
    case Message::CLIENT_TICK_DONE:
        client->ticksDone++;
        assert(client->ticksDone <= ticksStarted);
        return;

    default:
        return;
    }
}

int main() {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet" << std::endl;    
        return 1;
    }

    settings.randomSeed = static_cast<uint32_t>(time(NULL));
    settings.mapW = 256;
    settings.mapH = 256;
    settings.heightLimit = 15;
    settings.tickLengthMs = 50;

    size_t numWaitPlayers = 1;

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1234;

    server = enet_host_create(&address, 32, 2, 0, 0);

    if (server == NULL) {
        std::cerr << "Failed to create host." << std::endl;
        return 1;
    }

    std::cout << "Server started" << std::endl;

    bool quit = false;
    while (!quit) {
        if (!gameStarted && clients.size() == numWaitPlayers) {
            startGame();
            startTick();
            startTick();
        }

        if (gameStarted) {
            assert(ticksStarted >= 2);

            bool prevTickDone = true;
            for (auto client : clients) {
                assert(client->ticksDone <= ticksStarted);

                prevTickDone = prevTickDone && (client->ticksDone >= ticksStarted - 1);
            }

            if (prevTickDone)
                startTick();
        }

        ENetEvent event;

        while (enet_host_service(server, &event, 0) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                std::cout << "A new client connected" << std::endl;

                if (gameStarted) {
                    std::cout << "Rejecting client since game has started" << std::endl;
                    enet_peer_reset(event.peer);
                    // TODO: Cleanup ENet stuff?
                    break;
                }

                ClientInfo *client = new ClientInfo(++playerCounter, event.peer);
                client->player.color = playerCounter % 4;
                client->player.team = playerCounter;

                event.peer->data = client;

                clients.push_back(client);

                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                ClientInfo *client = static_cast<ClientInfo *>(event.peer->data);

                BitStreamReader reader(event.packet->data, event.packet->dataLength);

                Message::Type messageType;
                read(reader, messageType);
                Message message(messageType);
                read(reader, message);

                handleMessage(client, message);

                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                ClientInfo *client = static_cast<ClientInfo *>(event.peer->data);
                std::cout << "Player " << client->player.id << " disconnected" << std::endl;

                auto position = std::find(clients.begin(), clients.end(), client);
                assert(position != clients.end());
                clients.erase(position);

                delete client;

                if (clients.empty()) {
                    std::cout << "All players disconnected" << std::endl;
                    quit = true;
                }

                break;
            }

            default: assert(false);
            }
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();

    return 0;
}
