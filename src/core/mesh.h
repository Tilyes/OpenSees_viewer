#pragma once

#include <vector>
#include <cstdint>
#include <glm/vec3.hpp>

namespace viewer {

struct Node {
    uint32_t id;
    glm::vec3 position;
};

enum class ElementType : uint8_t {
    Line2 = 0,
    Tri3,
    Quad4,
    Tet4,
    Hex8,
    Custom
};

struct Element {
    uint32_t id;
    ElementType type;
    std::vector<uint32_t> node_ids;
};

class Mesh {
public:
    void add_node(uint32_t id, glm::vec3 pos);
    void add_element(uint32_t id, ElementType type, std::vector<uint32_t> node_ids);

    const std::vector<Node>& nodes() const { return nodes_; }
    const std::vector<Element>& elements() const { return elements_; }

    void clear();
    bool empty() const { return nodes_.empty(); }

private:
    std::vector<Node> nodes_;
    std::vector<Element> elements_;
};

} // namespace viewer
