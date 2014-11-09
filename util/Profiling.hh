#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

#define USE_PROFILING

#ifdef USE_PROFILING
#   define PROFILE(name) static ProfilingData _profilingData(#name); \
        ProfilingImpl _profilingImpl(_profilingData)
#else
#   define PROFILE(name) do {} while(0)
#endif

struct ProfilingData {
    char const* const name;

    int numCalls;
    double time;

    ProfilingData* const parent;
    std::vector<ProfilingData*> children;

    bool isRoot;

    ProfilingData(char const* name);

    static void reset();
    static void dump();

    static std::vector<ProfilingData*> roots;

private:
    friend struct ProfilingImpl;
    static ProfilingData* current; // the block we are currently in
};

// Updates given ProfilingData according to time elapsed
struct ProfilingImpl {
    ProfilingImpl(ProfilingData&);
    ~ProfilingImpl();

private:
    ProfilingData& data;
    double startTime;
};
