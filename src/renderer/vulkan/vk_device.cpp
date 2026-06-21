// =============================================================================
// 文件: renderer/vulkan/vk_device.cpp
// 作用: DeviceCtx 实现：首选独显、创建 Device、开启 VK_KHR_swapchain 扩展
// 被调用: VkContext (vk_context.h) 持有这三个子模块
// =============================================================================

#include "vk_device.h"
#include <spdlog/spdlog.h>

namespace viewer {

bool DeviceCtx::init(VkInstance instance, VkSurfaceKHR surface) {
    surface_ = surface; // 存下来，queue_families 查询要用

    if (!pick_physical_device(instance, surface)) {
        return false;
    }

    // 查询队列族
    QueueFamilyIndices indices = query_queue_families();

    // 创建逻辑设备
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphics.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physical_device_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
        spdlog::error("Failed to create logical device");
        return false;
    }

    vkGetDeviceQueue(device_, indices.graphics.value(), 0, &graphics_queue_);
    vkGetDeviceQueue(device_, indices.present.value(), 0, &present_queue_);

    return true;
}

void DeviceCtx::shutdown() {
    vkDestroyDevice(device_, nullptr);
}

bool DeviceCtx::pick_physical_device(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        spdlog::error("No GPU with Vulkan support found");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // 优先选独立显卡
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physical_device_ = device;
            spdlog::info("Selected GPU: {}", props.deviceName);
            return true;
        }
    }

    // fallback
    physical_device_ = devices[0];
    return true;
}

QueueFamilyIndices DeviceCtx::query_queue_families() const {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, i, surface_, &presentSupport);
        if (presentSupport) {
            indices.present = i;
        }

        if (indices.is_complete()) break;
    }

    return indices;
}

} // namespace viewer
