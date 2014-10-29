#include "Message.hh"

#include "BitStream.hh"

#include <cassert>

Message::Message(Message::Type type)
    : type(type) {
    switch (type) { // ugh...
    case Message::CLIENT_CONNECT:
        new(&client_connect) Message::ClientConnect;
        return;
    case Message::SERVER_TICK:
        new(&server_tick) Message::ServerTick;
        return;
    case Message::SERVER_START:
        new(&server_start.settings) GameSettings;
        return;
    default:
        return;
    }
}

Message::~Message() {
    switch (type) { // ugh...
    case Message::CLIENT_CONNECT:
        client_connect.~ClientConnect();
        return;
    case Message::SERVER_TICK:
        server_tick.~ServerTick();
        return;
    case Message::SERVER_START:
        server_start.~ServerStart();
        return;
    default:
        return;
    }
}

ENetPacket *Message::toPacket() const {
    BitStreamWriter writer;    
    write(writer, *this);

    return enet_packet_create(writer.ptr(),
                              writer.size(),
                              ENET_PACKET_FLAG_RELIABLE);
}

void read(BitStreamReader &reader, Message &message) {
    switch (message.type) {
    case Message::UNDEFINED:
        assert(false);
        return;
    case Message::CLIENT_CONNECT:
        read(reader, message.client_connect.name);
        return;
    case Message::CLIENT_ORDER:
        read(reader, message.client_order.order);
        return;
    case Message::CLIENT_TICK_DONE:
        return;
    case Message::SERVER_CONNECT:
        read(reader, message.server_connect.yourPlayerId);
        return;
    case Message::SERVER_TICK:
        read(reader, message.server_tick.orders);
        return;
    case Message::SERVER_START:
        read(reader, message.server_start.settings);
        return;
    }
}

void write(BitStreamWriter &writer, const Message &message) {
    write(writer, message.type);

    switch (message.type) {
    case Message::UNDEFINED:
        assert(false);
        return;
    case Message::CLIENT_CONNECT:
        write(writer, message.client_connect.name);
        return;
    case Message::CLIENT_ORDER:
        write(writer, message.client_order.order);
        return;
    case Message::CLIENT_TICK_DONE:
        return;
    case Message::SERVER_CONNECT:
        write(writer, message.server_connect.yourPlayerId);
        return;
    case Message::SERVER_TICK:
        write(writer, message.server_tick.orders);
        return;
    case Message::SERVER_START:
        write(writer, message.server_start.settings);
        return;
    }
}
