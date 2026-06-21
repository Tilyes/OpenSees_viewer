// =============================================================================
// 文件: utils/log.h
// 作用: Log：spdlog 封装，init() 初始化日志系统。全局单例
// =============================================================================

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
