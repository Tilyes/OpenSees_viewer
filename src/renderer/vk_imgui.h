// =============================================================================
// 文件: renderer/vk_imgui.h
// 作用: VkImGui：ImGui 和 Vulkan 的接线员。init 告诉 ImGui'用这份 Vulkan 环境画 UI'。new_frame→render 每帧把 UI 转成 Vulkan 指令
// =============================================================================

#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace viewer {

class VkContext;

class VkImGui {
public:
    bool init(VkContext& ctx, GLFWwindow* window);
    void shutdown();
    void new_frame();
    void render(VkCommandBuffer cmd);
};

} // namespace viewer
