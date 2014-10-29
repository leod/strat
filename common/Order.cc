#include "Order.hh"

#include "BitStream.hh"

#include <cassert>

void read(BitStreamReader &reader, Order &order) {
    read(reader, order.player);
    read(reader, order.type);

    switch (order.type) {
    case Order::UNDEFINED:
        assert(false);
        return;
    case Order::BUILD:
        read(reader, order.build.type);
        read(reader, order.build.x);
        read(reader, order.build.y);
        return;
    case Order::ATTACK:
        read(reader, order.attack.objectId);
        read(reader, order.attack.x);
        read(reader, order.attack.y);
        return;
    case Order::STOP:
        read(reader, order.stop.objectId);
        break;
    case Order::REMOVE:
        read(reader, order.remove.objectId);
        break;
    }
}

void write(BitStreamWriter &writer, const Order &order) {
    write(writer, order.player);
    write(writer, order.type);

    switch (order.type) {
    case Order::UNDEFINED:
        assert(false);
        return;
    case Order::BUILD:
        write(writer, order.build.type);
        write(writer, order.build.x);
        write(writer, order.build.y);
        return;
    case Order::ATTACK:
        write(writer, order.attack.objectId);
        write(writer, order.attack.x);
        write(writer, order.attack.y);
        return;
    case Order::STOP:
        write(writer, order.stop.objectId);
        break;
    case Order::REMOVE:
        write(writer, order.remove.objectId);
        break;
    }
}
