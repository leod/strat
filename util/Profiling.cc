#include "util/Profiling.hh"

#include <sstream>

#include "util/Log.hpp"

namespace game {

ProfilingData::ProfilingData(char const* name)
    : name(name), numCalls(0), time(), parent(current) {
    if (!current)
        roots.push_back(this);
    else
        current->children.push_back(this);
}

static void recReset(ProfilingData* data) {
    data->numCalls = 0;
    data->time = Time::Zero;

    for (auto child : data->children)
        recReset(child);
}

void ProfilingData::reset() {
    for (auto root : roots)
        recReset(root);
}

static void recDump(ProfilingData* data, Time total, int depth) {
    std::stringstream ss;

    for (int i = 0; i < depth; i++)
        ss << "  ";

    ss << data->name << ": " << data->numCalls << " calls, "
       << (float)data->time.asMilliseconds() / data->numCalls << "ms/call, "
       << (float)data->time.asMilliseconds() / total.asMilliseconds() * 100
       << "%";

    INFO(profiling) << ss.str();

    for (auto child : data->children)
        recDump(child, total, depth + 1);
}

void ProfilingData::dump() {
    Time total;
    for (auto root : roots)
        total += root->time;

    for (auto root : roots)
        recDump(root, total, 0);

    reset();
}

std::vector<ProfilingData*> ProfilingData::roots;
ProfilingData* ProfilingData::current = nullptr;

ProfilingImpl::ProfilingImpl(ProfilingData& data)
    : data(data) {
    if (ProfilingData::current != data.parent)
        WARN(profiling) << "Inconsistent profiling calls: " 
            << data.name << " was called first from "
            << (data.parent ? data.parent->name : "<root>")
            << " and then from "
            << (ProfilingData::current ?
                ProfilingData::current->name : "<root>");

    ProfilingData::current = &data;
}

ProfilingImpl::~ProfilingImpl() {
    data.numCalls++;
    data.time += clock.getElapsedTime();

    ProfilingData::current = data.parent;
}

} // namespace game
