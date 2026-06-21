// =============================================================================
// 文件: renderer/vk_buffer.h
// 作用: GpuBuffer：GPU 显存封装。create(分配VkBuffer+VkMemory) → upload(memcpy到显存) → destroy(释放)
// =============================================================================

#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace viewer {

class VkContext;

class GpuBuffer {
public:
    void create(VkContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroy(VkDevice device);
    void upload(VkDevice device, const void* data, VkDeviceSize size);

    VkBuffer_T* handle() const { return buffer_; }

private:
    uint32_t find_memory_type(VkPhysicalDevice pd, uint32_t filter, VkMemoryPropertyFlags props);

    VkBuffer_T* buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
};

} // namespace viewer
