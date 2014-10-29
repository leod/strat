#include "Client.hh"
#include "common/BitStream.hh"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>
#include <cassert>

Client::Client(const std::string &username)
    : username(username)
    , client(NULL)
    , peer(NULL)
    , sim(NULL)
    , playerId(0) 
    , tickRunning(false)
    , tickStartTime(0) {
}

Client::~Client() {
    if (client)
        enet_host_destroy(client);

    if (sim)
        delete sim;
}

void Client::connect(const std::string &host, int port) {
    std::cout << "Connecting to " << host << ":" << port << std::endl;

    client = enet_host_create(NULL, 1, 2, 0, 0);

    if (client == NULL)
        throw std::runtime_error("Failed to create ENet client");

    ENetAddress address;
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    peer = enet_host_connect(client, &address, 2, 0);

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connected!" << std::endl;
    } else {
        enet_peer_reset(peer);

        if (client == NULL)
            throw std::runtime_error("Failed to connect");
    }

    // Say hello
    Message message(Message::CLIENT_CONNECT);
    message.client_connect.name = username;
    sendMessage(message);
}

void Client::update() {
    if (tickRunning
        && glfwGetTime() - tickStartTime > settings.tickLengthMs / 1000.0f) {
        tickRunning = false;

        Message message(Message::CLIENT_TICK_DONE);
        sendMessage(message);
    }

    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            break;
        case ENET_EVENT_TYPE_RECEIVE: {
            BitStreamReader reader(event.packet->data, event.packet->dataLength);

            Message::Type messageType;
            read(reader, messageType);

            Message message(messageType);
            read(reader, message);

            handleMessage(message);

            enet_packet_destroy(event.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "Got disconnected" << std::endl;
            break;
        default: assert(false);
        }
    }
}

void Client::order(const Order &order) {
    assert(sim);

    if (sim->getState().isOrderValid(order)) {
        Message message(Message::CLIENT_ORDER);
        message.client_order.order = order;
        sendMessage(message);
    }
}

void Client::sendMessage(const Message &message) {
    ENetPacket *packet = message.toPacket();
    enet_peer_send(peer, 0, packet);
}

void Client::handleMessage(const Message &message) {
    switch (message.type) {
    case Message::SERVER_CONNECT:
        std::cout << "Connected to server with player id "
                  << message.server_connect.yourPlayerId << std::endl;
        playerId = message.server_connect.yourPlayerId;
        return;

    case Message::SERVER_START:
        std::cout << "Initializing simulation with seed "
                  << message.server_start.settings.randomSeed << std::endl;

        settings = message.server_start.settings;
        sim = new Sim(settings);
        return;

    case Message::SERVER_TICK:
        //std::cout << "Starting tick" << std::endl;

        if (sim) {
            sim->runTick(message.server_tick.orders);
            tickRunning = true;
            tickStartTime = glfwGetTime();
        }
        return;

    default:
        return;
    }
}
