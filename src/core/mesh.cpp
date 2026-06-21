// =============================================================================
// 文件: core/mesh.cpp
// 作用: Mesh 类实现：add_node(), add_element(), clear()
// 依赖方向: core/ 是最底层，不依赖任何其他模块
// =============================================================================

#include "mesh.h"

namespace viewer {

void Mesh::add_node(uint32_t id, glm::vec3 pos) {
    nodes_.push_back({id, pos});
}

void Mesh::add_element(uint32_t id, ElementType type, std::vector<uint32_t> node_ids) {
    elements_.push_back({id, type, std::move(node_ids)});
}

void Mesh::clear() {
    nodes_.clear();
    elements_.clear();
}

} // namespace viewer
