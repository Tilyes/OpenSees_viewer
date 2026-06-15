#pragma once

#include "mesh.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

namespace viewer {

struct ElementTopology {
    std::string name;
    uint32_t num_nodes;
    std::vector<std::vector<uint32_t>> faces;  // face connectivity (local node indices)
    std::vector<std::pair<uint32_t, uint32_t>> edges;  // edge connectivity
};

class ElementRegistry {
public:
    static ElementRegistry& instance();

    void register_type(ElementType type, ElementTopology topo);
    const ElementTopology* get_topology(ElementType type) const;

private:
    ElementRegistry();
    std::unordered_map<uint8_t, ElementTopology> registry_;
};

} // namespace viewer
