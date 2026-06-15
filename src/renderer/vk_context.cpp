#include "vk_context.h"
#include <spdlog/spdlog.h>

namespace viewer {

bool VkContext::init(GLFWwindow* window) {
    if (!instance_ctx_.init(window)) return false;
    if (!device_ctx_.init(instance_ctx_.instance(), instance_ctx_.surface())) return false;
    if (!swapchain_ctx_.init_phase1(device_ctx_.physical_device(), device_ctx_.device(),
                                    instance_ctx_.surface(), window)) return false;
    if (!create_render_pass()) return false;
    if (!swapchain_ctx_.init_phase2(device_ctx_.device(), render_pass_)) return false;
    if (!create_command_pool()) return false;
    spdlog::info("Vulkan context initialized");
    return true;
}

void VkContext::shutdown() {
    vkDeviceWaitIdle(device_ctx_.device());
    vkDestroyCommandPool(device_ctx_.device(), command_pool_, nullptr);
    swapchain_ctx_.shutdown(device_ctx_.device());
    vkDestroyRenderPass(device_ctx_.device(), render_pass_, nullptr);
    device_ctx_.shutdown();
    instance_ctx_.shutdown();
    spdlog::info("Vulkan context destroyed");
}

void VkContext::recreate_swapchain(GLFWwindow* window) {
    vkDeviceWaitIdle(device_ctx_.device());
    swapchain_ctx_.recreate(device_ctx_.physical_device(), device_ctx_.device(),
                            instance_ctx_.surface(), render_pass_, window);
}

bool VkContext::create_render_pass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain_ctx_.format();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device_ctx_.device(), &renderPassInfo, nullptr, &render_pass_) != VK_SUCCESS) {
        spdlog::error("Failed to create render pass");
        return false;
    }
    return true;
}

bool VkContext::create_command_pool() {
    QueueFamilyIndices indices = device_ctx_.query_queue_families();
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphics.value();

    if (vkCreateCommandPool(device_ctx_.device(), &poolInfo, nullptr, &command_pool_) != VK_SUCCESS) {
        spdlog::error("Failed to create command pool");
        return false;
    }
    return true;
}

} // namespace viewer
