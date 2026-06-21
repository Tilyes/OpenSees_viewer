// =============================================================================
// 文件: core/field.h
// 作用: Field 类：节点场/单元场，存标量/向量/张量结果数据
// 依赖方向: core/ 是最底层，不依赖任何其他模块
// =============================================================================

#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace viewer {

enum class FieldLocation : uint8_t {
    NodeField,
    ElementField
};

enum class FieldType : uint8_t {
    Scalar,
    Vector3,
    Tensor6  // symmetric tensor: xx, yy, zz, xy, yz, xz
};

struct Field {
    std::string name;
    FieldLocation location;
    FieldType type;
    std::vector<float> data;

    uint32_t components_per_entry() const;
};

} // namespace viewer
