#pragma once

#include <vulkan/vulkan.h>
#include <array>

namespace viewer {

class VkContext;

class Renderloop {
public:
    bool init(VkContext& ctx);
    bool shutdown(VkDevice device);
    bool begin_frame(VkContext& ctx);
    void end_frame(VkContext& ctx);
    VkCommandBuffer current_cmd() const;

private:
    static constexpr int MAX_SWAPCHAIN_IMAGES = 4;

    VkCommandBuffer cmd_;
    uint32_t current_image_index_;
    uint32_t render_finished_count_ = 0;   // 实际创建了几个

    VkSemaphore image_available_;
    std::array<VkSemaphore, MAX_SWAPCHAIN_IMAGES> render_finished_{}; // 每张图一个
    VkFence in_flight_;
};

} // namespace viewer
