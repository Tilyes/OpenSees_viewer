// =============================================================================
// 文件: renderer/vk_context.h
// 作用: VkContext：Vulkan 子系统协调者。持有 InstanceCtx + DeviceCtx + SwapchainCtx + RenderPass + CommandPool。对外暴露统一 getter
// =============================================================================

#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "vulkan/vk_instance.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_swapchain.h"

namespace viewer {

class VkContext {
public:
    bool init(GLFWwindow* window);
    void shutdown();
    void recreate_swapchain(GLFWwindow* window);

    VkInstance instance() const { return instance_ctx_.instance(); }
    VkSurfaceKHR surface() const { return instance_ctx_.surface(); }
    VkPhysicalDevice physical_device() const { return device_ctx_.physical_device(); }
    VkDevice device() const { return device_ctx_.device(); }
    VkQueue graphics_queue() const { return device_ctx_.graphics_queue(); }
    VkQueue present_queue() const { return device_ctx_.present_queue(); }
    QueueFamilyIndices queue_families() { return device_ctx_.query_queue_families(); }
    VkSwapchainKHR swapchain() const { return swapchain_ctx_.swapchain(); }
    VkFormat swapchain_format() const { return swapchain_ctx_.format(); }
    VkExtent2D swapchain_extent() const { return swapchain_ctx_.extent(); }
    const std::vector<VkImageView>& swapchain_image_views() const { return swapchain_ctx_.image_views(); }
    const std::vector<VkFramebuffer>& framebuffers() const { return swapchain_ctx_.framebuffers(); }
    VkRenderPass render_pass() const { return render_pass_; }
    VkCommandPool command_pool() const { return command_pool_; }

private:
    bool create_render_pass();
    bool create_command_pool();

    InstanceCtx instance_ctx_;
    DeviceCtx device_ctx_;
    SwapchainCtx swapchain_ctx_;
    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    VkCommandPool command_pool_ = VK_NULL_HANDLE;
};

} // namespace viewer
