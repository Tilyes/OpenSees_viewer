// =============================================================================
// 文件: core/model.cpp
// 作用: Model 类实现：set_mesh(), add_timestep()
// 依赖方向: core/ 是最底层，不依赖任何其他模块
// =============================================================================

#include "model.h"

namespace viewer {

void Model::set_mesh(Mesh mesh) {
    mesh_ = std::move(mesh);
}

void Model::add_timestep(TimeStep step) {
    timesteps_.push_back(std::move(step));
}

const TimeStep* Model::timestep_at(size_t index) const {
    if (index >= timesteps_.size()) return nullptr;
    return &timesteps_[index];
}

void Model::clear() {
    mesh_.clear();
    timesteps_.clear();
}

} // namespace viewer
