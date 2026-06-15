#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct GLFWwindow;

namespace viewer {

// 管理 Vulkan Instance 和 Window Surface
// 这是 Vulkan 和操作系统/窗口系统之间的桥梁
class InstanceCtx {
public:
    bool init(GLFWwindow* window);
    void shutdown();

    VkInstance instance() const { return instance_; }
    VkSurfaceKHR surface() const { return surface_; }

private:
    const std::vector<const char*> validation_layers_ = {
        "VK_LAYER_KHRONOS_validation"
    };
#ifdef _DEBUG
    bool enable_validation_layers_ = true;
#else
    bool enable_validation_layers_ = false;
#endif

    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
};

} // namespace viewer
