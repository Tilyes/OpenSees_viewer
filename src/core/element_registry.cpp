// =============================================================================
// 文件: core/element_registry.cpp
// 作用: ElementRegistry 实现
// 依赖方向: core/ 是最底层，不依赖任何其他模块
// =============================================================================

#include "element_registry.h"

namespace viewer {

ElementRegistry& ElementRegistry::instance() {
    static ElementRegistry reg;
    return reg;
}

ElementRegistry::ElementRegistry() {
    register_type(ElementType::Line2, {
        "Line2", 2, {}, {{0, 1}}
    });

    register_type(ElementType::Tri3, {
        "Tri3", 3,
        {{0, 1, 2}},
        {{0, 1}, {1, 2}, {2, 0}}
    });

    register_type(ElementType::Quad4, {
        "Quad4", 4,
        {{0, 1, 2, 3}},
        {{0, 1}, {1, 2}, {2, 3}, {3, 0}}
    });

    register_type(ElementType::Tet4, {
        "Tet4", 4,
        {{0, 1, 2}, {0, 1, 3}, {1, 2, 3}, {0, 2, 3}},
        {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {1, 3}, {2, 3}}
    });

    register_type(ElementType::Hex8, {
        "Hex8", 8,
        {{0,1,2,3}, {4,5,6,7}, {0,1,5,4}, {2,3,7,6}, {0,3,7,4}, {1,2,6,5}},
        {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}}
    });
}

void ElementRegistry::register_type(ElementType type, ElementTopology topo) {
    registry_[static_cast<uint8_t>(type)] = std::move(topo);
}

const ElementTopology* ElementRegistry::get_topology(ElementType type) const {
    auto it = registry_.find(static_cast<uint8_t>(type));
    if (it == registry_.end()) return nullptr;
    return &it->second;
}

} // namespace viewer
