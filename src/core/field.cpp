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
