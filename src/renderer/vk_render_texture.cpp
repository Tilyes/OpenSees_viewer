#include "vk_render_texture.h"
#include <imgui_impl_vulkan.h>
#include <spdlog/spdlog.h>

namespace viewer {

static uint32_t find_memory_type(VkPhysicalDevice pd, uint32_t filter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(pd, &mem_props);
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++)
        if ((filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & props) == props)
            return i;
    return 0;
}

bool RenderTexture::init(VkDevice device, VkPhysicalDevice pd,
                         VkExtent2D extent, VkRenderPass render_pass) {
    extent_ = extent;

    // 1. 创建离屏图像（颜色附件 + 可被 shader 采样）
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
    imageInfo.extent = { extent.width, extent.height, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(device, &imageInfo, nullptr, &image_) != VK_SUCCESS) {
        spdlog::error("Failed to create render texture image");
        return false;
    }

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device, image_, &memReq);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = find_memory_type(pd, memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory_) != VK_SUCCESS) {
        spdlog::error("Failed to allocate render texture memory");
        return false;
    }
    vkBindImageMemory(device, image_, memory_, 0);

    // 2. ImageView
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image_;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(device, &viewInfo, nullptr, &view_) != VK_SUCCESS) {
        spdlog::error("Failed to create render texture view");
        return false;
    }

    // 3. Framebuffer
    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = render_pass;
    fbInfo.attachmentCount = 1;
    fbInfo.pAttachments = &view_;
    fbInfo.width = extent.width;
    fbInfo.height = extent.height;
    fbInfo.layers = 1;
    if (vkCreateFramebuffer(device, &fbInfo, nullptr, &framebuffer_) != VK_SUCCESS) {
        spdlog::error("Failed to create render texture framebuffer");
        return false;
    }

    // 4. Sampler（ImGui 采样纹理时用的）
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler_) != VK_SUCCESS) {
        spdlog::error("Failed to create sampler");
        return false;
    }

    // 5. 注册到 ImGui（自动创建 descriptor set）
    descriptor_set_ = ImGui_ImplVulkan_AddTexture(sampler_, view_,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    if (!descriptor_set_) {
        spdlog::error("Failed to register texture with ImGui");
        return false;
    }

    return true;
}

void RenderTexture::shutdown(VkDevice device) {
    ImGui_ImplVulkan_RemoveTexture(descriptor_set_);
    vkDestroySampler(device, sampler_, nullptr);
    vkDestroyFramebuffer(device, framebuffer_, nullptr);
    vkDestroyImageView(device, view_, nullptr);
    vkDestroyImage(device, image_, nullptr);
    vkFreeMemory(device, memory_, nullptr);
}

} // namespace viewer
