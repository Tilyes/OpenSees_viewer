#pragma once

#include <string>
#include <memory>

namespace viewer {

// PIMPL 模式：app.h 完全不暴露 Vulkan/GLFW 头文件
// 所有实现细节藏在 app.cpp 的 App::Impl 里
class App {
public:
    App();
    ~App();

    bool init(const std::string& title, int width, int height);
    void run();
    void shutdown();

private:
    void main_loop();

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace viewer
