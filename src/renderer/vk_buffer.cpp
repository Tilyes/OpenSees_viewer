#include "vk_buffer.h"
#include "vk_context.h"
#include <spdlog/spdlog.h>
#include <cstring>  // memcpy

namespace viewer {

void GpuBuffer::create(VkContext& ctx, VkDeviceSize size,
                       VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    // 1. 创建 VkBuffer（只是一个句柄，还没有分配显存）
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(ctx.device(), &bufferInfo, nullptr, &buffer_) != VK_SUCCESS) {
        spdlog::error("Failed to create vertex buffer");
        return;
    }

    // 2. 查询这块 buffer 需要什么样（类型、对齐）的显存
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx.device(), buffer_, &memRequirements);

    // 3. 分配显存
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(
        ctx.physical_device(), memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(ctx.device(), &allocInfo, nullptr, &memory_) != VK_SUCCESS) {
        spdlog::error("Failed to allocate vertex buffer memory");
        return;
    }

    // 4. 把显存绑到 buffer 上
    vkBindBufferMemory(ctx.device(), buffer_, memory_, 0);
}

void GpuBuffer::destroy(VkDevice device) {
    vkDestroyBuffer(device, buffer_, nullptr);
    vkFreeMemory(device, memory_, nullptr);
}

void GpuBuffer::upload(VkDevice device, const void* data, VkDeviceSize size) {
    // 1. 把显存映射到 CPU 可写的地址空间
    void* mapped;
    vkMapMemory(device, memory_, 0, size, 0, &mapped);

    // 2. 把数据拷过去
    memcpy(mapped, data, size);

    // 3. 解除映射
    vkUnmapMemory(device, memory_);
}

uint32_t GpuBuffer::find_memory_type(VkPhysicalDevice pd, uint32_t filter,
                                     VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(pd, &mem_props);
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
        if ((filter & (1 << i)) &&
            (mem_props.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }
    return 0;
}

} // namespace viewer
