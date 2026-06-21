// =============================================================================
// 文件: main.cpp
// 作用: 程序入口，创建 App 并启动主循环
// 调用链: main → App::init → App::run → App::shutdown
// =============================================================================

#include "ui/app.h"
#include "utils/log.h"

int main() {
	// 定义APP入口函数，返回值为整数，参数为命令行参数
	// 仅用于初始化APP界面，不涉及vulkan和imgui的初始化，这些将在app.cpp中完成

	// 初始化日志系统
	// viewer是命名空间，Log是类，init是静态成员函数
    viewer::Log::init();

    viewer::App app;
    if (!app.init("OpenSees Viewer", 1600, 900)) {
        return -1;
    }

    app.run();
    app.shutdown();
    return 0;
}
