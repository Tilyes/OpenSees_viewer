#include "vk_imgui.h"
#include "vk_context.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace viewer {

bool VkImGui::init(VkContext& ctx, GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsResizeFromEdges = true;

    // ── Windows 11 Fluent 风格 ──
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding     = 8.0f;   // 窗口圆角
    style.FrameRounding      = 5.0f;   // 按钮/输入框圆角
    style.GrabRounding       = 5.0f;   // 滑块圆角
    style.PopupRounding      = 8.0f;
    style.ScrollbarRounding  = 10.0f;
    style.TabRounding        = 6.0f;
    style.WindowBorderSize   = 0.0f;   // 无边框更现代
    style.FrameBorderSize    = 0.0f;
    style.ItemSpacing        = ImVec2(8, 6);
    style.WindowPadding      = ImVec2(12, 12);
    style.FramePadding       = ImVec2(6, 4);
    style.IndentSpacing      = 22.0f;
    style.ScrollbarSize      = 14.0f;
    style.GrabMinSize        = 10.0f;
    style.Alpha              = 1.0f;

    // Win11 配色：深蓝灰底 + 浅蓝强调
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]             = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_Border]               = ImVec4(0.25f, 0.25f, 0.28f, 0.60f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_Header]               = ImVec4(0.22f, 0.22f, 0.25f, 0.80f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.32f, 0.32f, 0.36f, 1.00f);
    colors[ImGuiCol_Button]               = ImVec4(0.22f, 0.22f, 0.25f, 0.80f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.38f, 0.38f, 0.42f, 1.00f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.18f, 0.18f, 0.20f, 0.80f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.24f, 0.24f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.35f, 0.55f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.45f, 0.65f, 0.95f, 1.00f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.45f, 0.65f, 0.95f, 1.00f);
    colors[ImGuiCol_Tab]                  = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
    colors[ImGuiCol_TabActive]            = ImVec4(0.22f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_TabUnfocused]         = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_Separator]            = ImVec4(0.25f, 0.25f, 0.28f, 0.50f);
    colors[ImGuiCol_ResizeGrip]           = ImVec4(0.35f, 0.55f, 0.85f, 0.40f);
    colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.35f, 0.55f, 0.85f, 0.70f);
    colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.35f, 0.55f, 0.85f, 1.00f);

    // 初始化 GLFW 后端
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // 初始化 Vulkan 后端
    // DescriptorPoolSize > 0 让 ImGui 自己管理 descriptor pool，不需要手动创建
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_0;
    initInfo.Instance = ctx.instance();
    initInfo.PhysicalDevice = ctx.physical_device();
    initInfo.Device = ctx.device();
    initInfo.QueueFamily = ctx.queue_families().graphics.value();
    initInfo.Queue = ctx.graphics_queue();
    initInfo.DescriptorPoolSize = 64;   // ImGui 自动创建和管理 pool
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(ctx.swapchain_image_views().size());

    initInfo.PipelineInfoMain.RenderPass = ctx.render_pass();
    initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);

    return true;
}

void VkImGui::shutdown() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void VkImGui::new_frame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void VkImGui::render(VkCommandBuffer cmd) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

} // namespace viewer
