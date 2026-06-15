#pragma once

#include "mesh.h"
#include "field.h"
#include <vector>
#include <string>

namespace viewer {

struct TimeStep {
    double time;
    std::vector<Field> fields;
};

class Model {
public:
    void set_mesh(Mesh mesh);
    void add_timestep(TimeStep step);

    const Mesh& mesh() const { return mesh_; }
    const std::vector<TimeStep>& timesteps() const { return timesteps_; }
    size_t num_timesteps() const { return timesteps_.size(); }

    const TimeStep* timestep_at(size_t index) const;

    void clear();

private:
    Mesh mesh_;
    std::vector<TimeStep> timesteps_;
};

} // namespace viewer
