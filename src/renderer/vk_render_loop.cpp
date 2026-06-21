// =============================================================================
// 文件: renderer/vk_render_loop.cpp
// 作用: Renderloop 实现：每帧 8 步。同步对象：image_available_(GPU内部等画布), render_finished_(GPU等画完), in_flight_(CPU等GPU)
// =============================================================================

#include "vk_render_loop.h"
#include "vk_context.h"
#include <spdlog/spdlog.h>

namespace viewer {

bool Renderloop::init(VkContext& ctx) {
    // 分配命令缓冲区
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = ctx.command_pool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(ctx.device(), &allocInfo, &cmd_) != VK_SUCCESS) {
        spdlog::error("Failed to allocate command buffer");
        return false;
    }

    // 创建信号量：image_available 只需一个（每次 acquire 只用一次）
    // render_finished 每张 swapchain 图各一个（避免 Present 占用冲突）
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(ctx.device(), &semaphoreInfo, nullptr, &image_available_) != VK_SUCCESS ||
        vkCreateFence(ctx.device(), &fenceInfo, nullptr, &in_flight_) != VK_SUCCESS) {
        spdlog::error("Failed to create sync objects");
        return false;
    }

    render_finished_count_ = static_cast<uint32_t>(ctx.swapchain_image_views().size());
    for (uint32_t i = 0; i < render_finished_count_; i++) {
        if (vkCreateSemaphore(ctx.device(), &semaphoreInfo, nullptr, &render_finished_[i]) != VK_SUCCESS) {
            spdlog::error("Failed to create render_finished semaphore {}", i);
            return false;
        }
    }

    return true;
}

bool Renderloop::shutdown(VkDevice device) {
    vkDestroySemaphore(device, image_available_, nullptr);
    for (uint32_t i = 0; i < render_finished_count_; i++) {
        vkDestroySemaphore(device, render_finished_[i], nullptr);
    }
    vkDestroyFence(device, in_flight_, nullptr);
    return true;
}

VkCommandBuffer Renderloop::current_cmd() const {
    return cmd_;
}

bool Renderloop::begin_frame(VkContext& ctx) {
    // 1. 等上帧干完
    vkWaitForFences(ctx.device(), 1, &in_flight_, VK_TRUE, UINT64_MAX);

    // 2. 拿新图
    VkResult result = vkAcquireNextImageKHR(
        ctx.device(), ctx.swapchain(), UINT64_MAX,
        image_available_, VK_NULL_HANDLE, &current_image_index_);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) return false;

    // 3. 重置
    vkResetFences(ctx.device(), 1, &in_flight_);
    vkResetCommandBuffer(cmd_, 0);

    // 4. 开始录指令
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd_, &beginInfo);

    // 5. 开始画（清屏）
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ctx.render_pass();
    renderPassInfo.framebuffer = ctx.framebuffers()[current_image_index_];
    renderPassInfo.renderArea.extent = ctx.swapchain_extent();
    VkClearValue clearColor{};
    clearColor.color.float32[0] = 0.0f;
    clearColor.color.float32[1] = 0.0f;
    clearColor.color.float32[2] = 0.0f;
    clearColor.color.float32[3] = 1.0f;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(cmd_, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    return true;
}

void Renderloop::end_frame(VkContext& ctx) {
    // 6. 结束画
    vkCmdEndRenderPass(cmd_);
    vkEndCommandBuffer(cmd_);

    // 7. 提交给 GPU
    VkSemaphore waitSemaphores[] = { image_available_ };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { render_finished_[current_image_index_] };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd_;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    vkQueueSubmit(ctx.graphics_queue(), 1, &submitInfo, in_flight_);

    // 8. 展示
    VkSwapchainKHR swapchain = ctx.swapchain();
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &current_image_index_;
    vkQueuePresentKHR(ctx.present_queue(), &presentInfo);
}

} // namespace viewer
