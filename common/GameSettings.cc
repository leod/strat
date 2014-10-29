#include "GameSettings.hh"

#include "BitStream.hh"

void read(BitStreamReader &reader, PlayerInfo &player) {
    read(reader, player.id);
    read(reader, player.name);
    read(reader, player.team);
    read(reader, player.color);
}

void write(BitStreamWriter &writer, const PlayerInfo &player) {
    write(writer, player.id);
    write(writer, player.name);
    write(writer, player.team);
    write(writer, player.color);
}

void read(BitStreamReader &reader, GameSettings &settings) {
    read(reader, settings.players);
    read(reader, settings.randomSeed);
    read(reader, settings.mapW);
    read(reader, settings.mapH);
    read(reader, settings.heightLimit);
    read(reader, settings.tickLengthMs);
}

void write(BitStreamWriter &writer, const GameSettings &settings) {
    write(writer, settings.players);
    write(writer, settings.randomSeed);
    write(writer, settings.mapW);
    write(writer, settings.mapH);
    write(writer, settings.heightLimit);
    write(writer, settings.tickLengthMs);
}
