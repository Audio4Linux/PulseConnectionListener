#ifndef UTIL_HPP
#define UTIL_HPP

#include <glib-object.h>
#include <glib.h>
#include <string>

namespace util {

inline void debug(const std::string& s) {
    g_debug(s.c_str(), "%s");
}

inline void error(const std::string& s) {
    g_error(s.c_str(), "%s");
}

inline void critical(const std::string& s) {
    g_critical(s.c_str(), "%s");
}

inline void warning(const std::string& s) {
    g_warning(s.c_str(), "%s");
}

inline void info(const std::string& s) {
    g_info(s.c_str(), "%s");
}

}  // namespace util


#endif

