#pragma once

#include <SFML/System.hpp>

#include "core/Time.hpp"

#define USE_PROFILING

namespace game {

#ifdef USE_PROFILING
#   define PROFILE(name) static ProfilingData _profilingData(#name); \
        ProfilingImpl _profilingImpl(_profilingData)
#else
#   define PROFILE(name) do {} while(0)
#endif

struct ProfilingData {
    char const* const name;

    int numCalls;
    Time time;

    ProfilingData* const parent;
    std::vector<ProfilingData*> children;

    ProfilingData(char const* name);

    static void reset();
    static void dump();

    static std::vector<ProfilingData*> roots;

private:
    friend struct ProfilingImpl;
    static ProfilingData* current;
};

struct ProfilingImpl {
    ProfilingImpl(ProfilingData&);
    ~ProfilingImpl();

private:
    ProfilingData& data;
    Clock clock;
};

} // namespace game
