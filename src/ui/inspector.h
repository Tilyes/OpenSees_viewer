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
