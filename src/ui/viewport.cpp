#include "viewport.h"
#include "vk_context.h"
#include <imgui.h>
#include <spdlog/spdlog.h>

namespace viewer {

bool Viewport::init(VkContext& ctx) {
    VkExtent2D extent = ctx.swapchain_extent();
    if (!texture.init(ctx.device(), ctx.physical_device(), extent, ctx.render_pass())) {
        spdlog::error("Failed to create viewport texture");
        return false;
    }
    return true;
}

void Viewport::shutdown(VkDevice device) {
    texture.shutdown(device);
}

void Viewport::render_ui(ImVec2 pos, ImVec2 size) {
    ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
    {
        hovered_ = ImGui::IsWindowHovered();

        // 把渲染纹理显示出来（画满窗口）
        ImGui::Image(texture.imgui_texture(),
                     ImGui::GetContentRegionAvail(),
                     ImVec2(0, 0), ImVec2(1, 1));
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace viewer
