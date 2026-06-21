// =============================================================================
// 文件: renderer/vulkan/vk_instance.cpp
// 作用: InstanceCtx 实现：vkCreateInstance → vkCreateSurface，Debug 模式开验证层
// 被调用: VkContext (vk_context.h) 持有这三个子模块
// =============================================================================

#include "vk_instance.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace viewer {

bool InstanceCtx::init(GLFWwindow* window) {
    // 1. 填写应用信息
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "OpenSees_viewer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // 2. 获取 GLFW 需要的扩展
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // 3. 创建 Instance
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    if (enable_validation_layers_) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
        createInfo.ppEnabledLayerNames = validation_layers_.data();
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
        spdlog::error("Failed to create Vulkan instance");
        return false;
    }

    // 4. 创建 Surface（和窗口绑定）
    if (glfwCreateWindowSurface(instance_, window, nullptr, &surface_) != VK_SUCCESS) {
        spdlog::error("Failed to create window surface");
        return false;
    }

    return true;
}

void InstanceCtx::shutdown() {
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);
}

} // namespace viewer
