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
