#pragma once

#include <cstddef>

struct ImVec2;

namespace viewer {

class Timeline {
public:
    void render(ImVec2 pos, ImVec2 size);

    void set_total_steps(size_t n) { total_steps_ = n; }
    size_t current_step() const { return current_step_; }

private:
    size_t current_step_ = 0;
    size_t total_steps_  = 0;
    bool playing_        = false;
};

} // namespace viewer
