#include "opensees_source.h"
#include <spdlog/spdlog.h>

namespace viewer {

bool OpenSeesSource::load(const std::string& path, Model& out) {
    // TODO: link against OpenSees library, read domain directly
    spdlog::info("OpenSeesSource::load not yet implemented");
    return false;
}

} // namespace viewer
