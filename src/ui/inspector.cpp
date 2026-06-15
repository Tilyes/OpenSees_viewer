#include "inspector.h"
#include <imgui.h>

namespace viewer {

void Inspector::render(ImVec2 pos, ImVec2 size) {
    ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

    ImGui::Begin("Inspector");
    {
        if (selection_info_.empty())
            ImGui::TextDisabled("Click a node or element to inspect");
        else
            ImGui::TextUnformatted(selection_info_.c_str());

        if (ImGui::CollapsingHeader("Model Info", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BulletText("Nodes: --");
            ImGui::BulletText("Elements: --");
            ImGui::BulletText("Time steps: --");
        }
    }
    ImGui::End();
}

} // namespace viewer
