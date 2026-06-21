// =============================================================================
// 文件: adapter/data_source.h
// 作用: DataSource 抽象基类：定义 load(path) → Model 接口，所有数据源的统一入口
// 依赖: viewer_core (Mesh, Model, Field)
// =============================================================================

#pragma once

#include "model.h"
#include <string>


namespace viewer {

class DataSource {
public:
    virtual ~DataSource() = default;
    virtual bool load(const std::string& path, Model& out) = 0;
    virtual std::string name() const = 0;
};

} // namespace viewer
