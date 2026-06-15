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
