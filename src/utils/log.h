#pragma once

#include <spdlog/spdlog.h>

namespace viewer {

class Log {
public:
    static void init() {
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
    }
};

} // namespace viewer
