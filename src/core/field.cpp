// =============================================================================
// 文件: core/field.cpp
// 作用: Field 类实现
// 依赖方向: core/ 是最底层，不依赖任何其他模块
// =============================================================================

#include "field.h"

namespace viewer {

uint32_t Field::components_per_entry() const {
    switch (type) {
        case FieldType::Scalar:  return 1;
        case FieldType::Vector3: return 3;
        case FieldType::Tensor6: return 6;
    }
    return 0;
}

} // namespace viewer
