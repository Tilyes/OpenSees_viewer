// =============================================================================
// 文件: ui/timeline.cpp
// 作用: Timeline 实现：render 播放按钮 + SliderInt 时间步选择
// =============================================================================

#include "timeline.h"
#include <imgui.h>

namespace viewer {

void Timeline::render(ImVec2 pos, ImVec2 size) {
    ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

    ImGui::Begin("Timeline");
    {
        if (ImGui::Button(playing_ ? "Pause" : "Play"))
            playing_ = !playing_;

        ImGui::SameLine();
        if (total_steps_ > 0) {
            int cur = static_cast<int>(current_step_);
            ImGui::SliderInt("Step", &cur, 0, static_cast<int>(total_steps_ - 1));
            current_step_ = static_cast<size_t>(cur);
        } else {
            ImGui::TextDisabled("No time steps loaded");
        }
        ImGui::Text("Step %zu / %zu", current_step_, total_steps_);
    }
    ImGui::End();
}

} // namespace viewer
