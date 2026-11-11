#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace task4 {
[[noreturn]] inline void ensureFailed(const char *expression, const char *file,
                                      int line, std::string_view message = {}) {
    std::string diagnostic;
    if (!message.empty())
        diagnostic = std::string(message) + ": ";
    throw std::runtime_error(diagnostic + "ENSURE(" + expression +
                             ") failed at " + file + ":" +
                             std::to_string(line));
}
} // namespace task4

#define ENSURE_MSG(expr, message)                                              \
    do {                                                                       \
        if (!(expr))                                                           \
            ::task4::ensureFailed(#expr, __FILE__, __LINE__, (message));       \
    } while (false)

#define ENSURE(expr) ENSURE_MSG(expr, "")
