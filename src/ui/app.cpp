#include "app.h"

#include "vk_context.h"
#include "vk_imgui.h"
#include "vk_mesh_renderer.h"
#include "vk_render_loop.h"
#include "viewport.h"
#include "inspector.h"
#include "timeline.h"
#include "camera.h"
#include "file_source.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

namespace viewer {

struct App::Impl {
    GLFWwindow* window = nullptr;
    int width = 1600, height = 900;
    bool shutdown_complete = false;

    std::unique_ptr<VkContext> vk_ctx;
    std::unique_ptr<VkImGui> vk_imgui;
    std::unique_ptr<VkMeshRenderer> mesh_renderer;
    std::unique_ptr<Renderloop> render_loop;

    Viewport viewport;
    Inspector inspector;
    Timeline timeline;
    Model model_;

    // 离屏渲染用
    VkCommandBuffer vp_cmd  = VK_NULL_HANDLE;
    VkFence         vp_fence = VK_NULL_HANDLE;
};

App::App() : impl_(std::make_unique<Impl>()) {}
App::~App() { shutdown(); }

bool App::init(const std::string& title, int width, int height) {
    impl_->width = width; impl_->height = height;

    if (!glfwInit()) { spdlog::error("GLFW init failed"); return false; }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    impl_->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!impl_->window) { spdlog::error("Window creation failed"); return false; }

    impl_->vk_ctx = std::make_unique<VkContext>();
    if (!impl_->vk_ctx->init(impl_->window)) return false;

    impl_->vk_imgui = std::make_unique<VkImGui>();
    if (!impl_->vk_imgui->init(*impl_->vk_ctx, impl_->window)) return false;

    impl_->render_loop = std::make_unique<Renderloop>();
    if (!impl_->render_loop->init(*impl_->vk_ctx)) return false;

    impl_->mesh_renderer = std::make_unique<VkMeshRenderer>();
    if (!impl_->mesh_renderer->init(*impl_->vk_ctx)) return false;

    // 视口（离屏渲染目标）
    if (!impl_->viewport.init(*impl_->vk_ctx)) return false;

    // 离屏渲染用的命令缓冲区和 fence
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = impl_->vk_ctx->command_pool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(impl_->vk_ctx->device(), &allocInfo, &impl_->vp_cmd);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // 第一帧直接过，不等待
    vkCreateFence(impl_->vk_ctx->device(), &fenceInfo, nullptr, &impl_->vp_fence);

    spdlog::info("App initialized: {}x{}", width, height);
    return true;
}

void App::run() {
    while (!glfwWindowShouldClose(impl_->window)) {
        glfwPollEvents();
        main_loop();
    }
    vkDeviceWaitIdle(impl_->vk_ctx->device());
}

void App::main_loop() {
    VkDevice device  = impl_->vk_ctx->device();
    VkExtent2D extent = impl_->vk_ctx->swapchain_extent();

    // ── 第一阶段：CPU 攒 UI 数据 ──
    impl_->vk_imgui->new_frame();

    // 菜单栏
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Model...")) {
                FileSource fs;
                Model temp;
                if (fs.load("../example/exam1/simple.tcl", temp)) {
                    impl_->model_ = std::move(temp);
                    impl_->mesh_renderer->upload_mesh(*impl_->vk_ctx, impl_->model_.mesh());

                    // 自动适配相机到模型包围盒
                    auto& nodes = impl_->model_.mesh().nodes();
                    if (!nodes.empty()) {
                        glm::vec3 minPos = nodes[0].position;
                        glm::vec3 maxPos = nodes[0].position;
                        for (auto& n : nodes) {
                            minPos = glm::min(minPos, n.position);
                            maxPos = glm::max(maxPos, n.position);
                        }
                        glm::vec3 center = (minPos + maxPos) * 0.5f;
                        float size = glm::distance(minPos, maxPos);
                        impl_->viewport.camera.set_target(center);
                        impl_->viewport.camera.set_distance(size * 0.7f);
                        spdlog::info("Camera auto-fit: center=({:.0f},{:.0f},{:.0f}) size={:.0f}",
                                     center.x, center.y, center.z, size);
                    }
                }
            }
            ImGui::MenuItem("Exit");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Wireframe");
            ImGui::MenuItem("Solid");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // 布局计算
    ImGuiViewport* vp = ImGui::GetMainViewport();
    float menu_h  = ImGui::GetFrameHeight();
    float insp_w  = 280.0f;
    float time_h  = 120.0f;
    float avail_w = vp->WorkSize.x;
    float avail_h = vp->WorkSize.y - menu_h;
    float view_w  = avail_w - insp_w;
    float view_h  = avail_h - time_h;

    // ImGui 面板（先渲染 UI 逻辑，后续会覆盖 3D 画面）
    impl_->viewport.render_ui(ImVec2(vp->WorkPos.x, vp->WorkPos.y + menu_h),
                              ImVec2(view_w, view_h));
    impl_->inspector.render(ImVec2(vp->WorkPos.x + view_w, vp->WorkPos.y + menu_h),
                            ImVec2(insp_w, view_h));
    impl_->timeline.render(ImVec2(vp->WorkPos.x, vp->WorkPos.y + menu_h + view_h),
                           ImVec2(avail_w, time_h));

    // 相机控制：只要鼠标在视口上就响应（不管 WantCaptureMouse）
    if (impl_->viewport.is_hovered()) {
        static double lx = 0, ly = 0;
        double mx, my;
        glfwGetCursorPos(impl_->window, &mx, &my);
        if (glfwGetMouseButton(impl_->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            impl_->viewport.camera.orbit(static_cast<float>(mx - lx),
                                         static_cast<float>(my - ly));
        lx = mx; ly = my;
        float scroll = ImGui::GetIO().MouseWheel;
        if (scroll != 0.0f) impl_->viewport.camera.zoom(scroll);
    }

    // ── 第二阶段：渲染 3D 到离屏纹理 ──
    VkCommandBuffer vcmd = impl_->vp_cmd;
    vkWaitForFences(device, 1, &impl_->vp_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &impl_->vp_fence);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(vcmd, &beginInfo);

    // 画到 Viewport 的离屏 framebuffer
    {
        VkRenderPassBeginInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpInfo.renderPass = impl_->vk_ctx->render_pass();
        rpInfo.framebuffer = impl_->viewport.texture.framebuffer();
        rpInfo.renderArea.extent = extent;
        VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
        rpInfo.clearValueCount = 1;
        rpInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(vcmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

        // 每个视口用自己的相机矩阵
        UniformBufferObject ubo{};
        ubo.model = glm::mat4(1.0f);
        ubo.view  = impl_->viewport.camera.view_matrix();
        ubo.proj  = impl_->viewport.camera.projection_matrix(
            static_cast<float>(extent.width) / extent.height);
        impl_->mesh_renderer->render(vcmd, extent, ubo);

        vkCmdEndRenderPass(vcmd);

        // 布局转换：COLOR_ATTACHMENT → SHADER_READ_ONLY（ImGui 采样需要）
        VkImageMemoryBarrier imgBarrier{};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // render pass finalLayout
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // ImGui 采样
        imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imgBarrier.image = impl_->viewport.texture.image();
        imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgBarrier.subresourceRange.levelCount = 1;
        imgBarrier.subresourceRange.layerCount = 1;
        imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(vcmd,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &imgBarrier);
    }

    vkEndCommandBuffer(vcmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vcmd;
    vkQueueSubmit(impl_->vk_ctx->graphics_queue(), 1, &submitInfo, impl_->vp_fence);

    // ── 第三阶段：渲染 ImGui 到 swapchain ──
    if (!impl_->render_loop->begin_frame(*impl_->vk_ctx)) {
        impl_->vk_ctx->recreate_swapchain(impl_->window);
        ImGui::EndFrame();
        return;
    }
    impl_->vk_imgui->render(impl_->render_loop->current_cmd());
    impl_->render_loop->end_frame(*impl_->vk_ctx);
}

void App::shutdown() {
    if (impl_->shutdown_complete) return;
    impl_->shutdown_complete = true;

    VkDevice device = impl_->vk_ctx ? impl_->vk_ctx->device() : VK_NULL_HANDLE;
    if (device) vkDeviceWaitIdle(device);

    vkDestroyFence(device, impl_->vp_fence, nullptr);
    impl_->viewport.shutdown(device);
    if (impl_->mesh_renderer) impl_->mesh_renderer->shutdown(*impl_->vk_ctx);
    if (impl_->render_loop) impl_->render_loop->shutdown(device);
    if (impl_->vk_imgui) impl_->vk_imgui->shutdown();
    if (impl_->vk_ctx) impl_->vk_ctx->shutdown();
    if (impl_->window) { glfwDestroyWindow(impl_->window); impl_->window = nullptr; }
    glfwTerminate();
    spdlog::info("App shutdown complete");
}

} // namespace viewer
