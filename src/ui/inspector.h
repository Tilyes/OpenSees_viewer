// =============================================================================
// 文件: ui/inspector.h
// 作用: Inspector：属性面板。显示选中节点/单元的详细信息（ImGui 纯 UI，不碰 Vulkan）
// =============================================================================

#pragma once

#include <string>

struct ImVec2;

namespace viewer {

class Inspector {
public:
    void render(ImVec2 pos, ImVec2 size);
    void set_selection(const std::string& info) { selection_info_ = info; }

private:
    std::string selection_info_;
};

} // namespace viewer
