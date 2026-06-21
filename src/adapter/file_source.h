// =============================================================================
// 文件: adapter/file_source.h
// 作用: FileSource：从 TCL/JSON/VTK 文件解析模型
// 依赖: viewer_core (Mesh, Model, Field)
// =============================================================================

#pragma once

#include "data_source.h"

namespace viewer {

class FileSource : public DataSource {
public:
    bool load(const std::string& path, Model& out) override;
    std::string name() const override { return "FileSource"; }
};

} // namespace viewer
