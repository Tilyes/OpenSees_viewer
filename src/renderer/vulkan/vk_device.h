// =============================================================================
// 文件: renderer/vulkan/vk_device.h
// 作用: DeviceCtx：选择物理显卡(GPU) → 创建逻辑设备 → 获取队列句柄。QueueFamilyIndices 结构体定义在此
// 被调用: VkContext (vk_context.h) 持有这三个子模块
// =============================================================================

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace viewer {

// 队列族索引：一个队列族可以支持多种操作类型
struct QueueFamilyIndices {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;
    bool is_complete() const { return graphics.has_value() && present.has_value(); }
};

// 管理物理设备选择和逻辑设备创建
// 封装 GPU 挑选策略、设备特性协商、队列获取
class DeviceCtx {
public:
    bool init(VkInstance instance, VkSurfaceKHR surface);
    void shutdown();

    VkPhysicalDevice physical_device() const { return physical_device_; }
    VkDevice device() const { return device_; }
    VkQueue graphics_queue() const { return graphics_queue_; }
    VkQueue present_queue() const { return present_queue_; }

    // 查询队列族（逻辑设备创建前后都可能需要调用）
    QueueFamilyIndices query_queue_families() const;

private:
    bool pick_physical_device(VkInstance instance, VkSurfaceKHR surface);

    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSurfaceKHR surface_ = VK_NULL_HANDLE; // 查 present support 需要，不拥有
};

} // namespace viewer
