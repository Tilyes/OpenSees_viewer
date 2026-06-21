// =============================================================================
// 文件: renderer/vulkan/vk_swapchain.h
// 作用: SwapchainCtx：双缓冲画布系统，管理 Swapchain + ImageViews + Framebuffers。窗口 resize 时重建
// 被调用: VkContext (vk_context.h) 持有这三个子模块
// =============================================================================

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct GLFWwindow;

namespace viewer {

// 管理 Swapchain 及其关联资源（ImageViews、Framebuffers）
// 窗口 resize 时整个替换这个对象即可
class SwapchainCtx {
public:
    // 第一阶段：创建 swapchain + image_views（format 可用）
    bool init_phase1(VkPhysicalDevice physical_device, VkDevice device,
                     VkSurfaceKHR surface, GLFWwindow* window);
    // 第二阶段：创建 framebuffers（需要 render_pass）
    bool init_phase2(VkDevice device, VkRenderPass render_pass);

    void shutdown(VkDevice device);
    void recreate(VkPhysicalDevice physical_device, VkDevice device,
                  VkSurfaceKHR surface, VkRenderPass render_pass,
                  GLFWwindow* window);

    VkSwapchainKHR swapchain() const { return swapchain_; }
    VkFormat format() const { return format_; }
    VkExtent2D extent() const { return extent_; }
    uint32_t image_count() const { return static_cast<uint32_t>(image_views_.size()); }
    VkImageView image_view(uint32_t i) const { return image_views_[i]; }
    VkFramebuffer framebuffer(uint32_t i) const { return framebuffers_[i]; }
    const std::vector<VkImageView>& image_views() const { return image_views_; }
    const std::vector<VkFramebuffer>& framebuffers() const { return framebuffers_; }

private:
    bool create_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                          VkSurfaceKHR surface, GLFWwindow* window);
    bool create_image_views(VkDevice device);
    bool create_framebuffers(VkDevice device, VkRenderPass render_pass);

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat format_{};
    VkExtent2D extent_{};
    std::vector<VkImage> images_;
    std::vector<VkImageView> image_views_;
    std::vector<VkFramebuffer> framebuffers_;
};

} // namespace viewer
