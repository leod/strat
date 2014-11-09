#include "util/Profiling.hh"

#include <sstream>
#include <algorithm>

#include "util/Log.hh"

ProfilingData::ProfilingData(char const* name)
    : name(name), numCalls(0), time(0), parent(current), isRoot(!current) {
    if (!current)
        roots.push_back(this);
    else
        current->children.push_back(this);
}

static void recReset(ProfilingData* data) {
    data->numCalls = 0;
    data->time = 0;

    for (auto child : data->children)
        recReset(child);
}

void ProfilingData::reset() {
    for (auto root : roots)
        recReset(root);
}

static void recDump(ProfilingData* data, double total, int depth) {
    std::stringstream ss;

    for (int i = 0; i < depth; i++)
        ss << "  ";

    ss << data->name << ": " << data->numCalls << " calls, "
       << data->time * 1000.0f / data->numCalls << "ms/call, "
       << data->time / total * 100.0f
       << "%";

    INFO(profiling) << ss.str();

    for (auto child : data->children)
        recDump(child, total, depth + 1);
}

void ProfilingData::dump() {
    double total = 0;
    for (auto root : roots)
        total += root->time;

    for (auto root : roots)
        recDump(root, total, 0);

    reset();
}

std::vector<ProfilingData*> ProfilingData::roots;
ProfilingData* ProfilingData::current = nullptr;

ProfilingImpl::ProfilingImpl(ProfilingData& data)
    : data(data), startTime(glfwGetTime()) {
    /*if (ProfilingData::current != data.parent)
        WARN(profiling) << "Inconsistent profiling calls: " 
            << data.name << " was called first from "
            << (data.parent ? data.parent->name : "<root>")
            << " and then from "
            << (ProfilingData::current ?
                ProfilingData::current->name : "<root>");*/

    if (ProfilingData::current && data.isRoot) {
        auto it = std::find(ProfilingData::roots.begin(), ProfilingData::roots.end(), &data);
        assert(it != ProfilingData::roots.end());
        ProfilingData::roots.erase(it);

        ProfilingData::current->children.push_back(&data);

        data.isRoot = false;
    }

    ProfilingData::current = &data;
}

ProfilingImpl::~ProfilingImpl() {
    data.numCalls++;
    data.time += glfwGetTime() - startTime;

    ProfilingData::current = data.parent;
}
