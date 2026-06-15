#pragma once

#include "data_source.h"

namespace viewer {

class FileSource : public DataSource {
public:
    bool load(const std::string& path, Model& out) override;
    std::string name() const override { return "FileSource"; }
};

} // namespace viewer
