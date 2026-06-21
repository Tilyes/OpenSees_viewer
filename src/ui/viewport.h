// =============================================================================
// 文件: ui/viewport.h
// 作用: Viewport：3D 视口面板。持有 RenderTexture + Camera。init 创建离屏纹理，render_ui 用 ImGui::Image 显示 3D 画面
// =============================================================================

#pragma once

#include "vk_render_texture.h"
#include "vk_mesh_renderer.h"
#include "camera.h"

struct ImVec2;

namespace viewer {

class VkContext;

class Viewport {
public:
    bool init(VkContext& ctx);
    void shutdown(VkDevice device);

    // 画 UI（用 ImGui::Image 显示渲染纹理）
    void render_ui(ImVec2 pos, ImVec2 size);
    bool is_hovered() const { return hovered_; }

    // 每个视口有自己独立的相机
    Camera camera;
    RenderTexture texture;

private:
    bool hovered_ = false;
};

} // namespace viewer
