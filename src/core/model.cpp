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
