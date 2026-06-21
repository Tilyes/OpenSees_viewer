// =============================================================================
// 文件: renderer/vk_render_texture.h
// 作用: RenderTexture：离屏渲染目标。创建 Image→Framebuffer→Sampler，注册为 ImGui 纹理。Viewport 用它做 3D→纹理→ImGui::Image 显示
// =============================================================================

#pragma once

#include <vulkan/vulkan.h>
#include <imgui.h>  // ImTextureID

namespace viewer {

// 离屏渲染目标：一个可以画上去的纹理，ImGui 可以显示它
class RenderTexture {
public:
    bool init(VkDevice device, VkPhysicalDevice pd, VkExtent2D extent, VkRenderPass render_pass);
    void shutdown(VkDevice device);

    VkFramebuffer framebuffer() const { return framebuffer_; }
    VkExtent2D extent() const { return extent_; }
    VkImage image() const { return image_; }

    // 给 ImGui::Image() 用的纹理 ID（内部是 descriptor set）
    ImTextureID imgui_texture() const { return reinterpret_cast<ImTextureID>(descriptor_set_); }

private:
    VkImage image_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    VkImageView view_ = VK_NULL_HANDLE;
    VkSampler sampler_ = VK_NULL_HANDLE;
    VkFramebuffer framebuffer_ = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
    VkExtent2D extent_{};
};

} // namespace viewer
