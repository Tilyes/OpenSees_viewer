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
