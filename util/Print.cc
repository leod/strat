#include "util/Print.hh"

#ifdef USING_GCC
#include <cxxabi.h>
#endif

namespace game {

std::string demangle(std::string const& s, bool qualified) {
#ifdef USING_GCC
    int status;
    char* name = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
    std::string result(name);
    free(name);

    if (!qualified) {
        if (result.substr(0, std::string("game::").size()) == "game::")
            return result.substr(std::string("game::").size());
    }

    return result;
#else
    return s;
#endif
}

} // namespace game
