// =============================================================================
// 文件: adapter/opensees_source.h
// 作用: OpenSeesSource：直接链接 OpenSees 库（stub，待实现）
// 依赖: viewer_core (Mesh, Model, Field)
// =============================================================================

#pragma once

#include "data_source.h"

namespace viewer {

class OpenSeesSource : public DataSource {
public:
    bool load(const std::string& path, Model& out) override;
    std::string name() const override { return "OpenSeesSource"; }
};

} // namespace viewer
