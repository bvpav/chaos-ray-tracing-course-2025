#pragma once

#include <string>

namespace crt {

enum class MaterialType {
    Diffuse,
    Reflective,
    Refractive,
    Constant,
};

struct Material {
    MaterialType type;
    int albedo_map_texture_index;
    float ior;
    bool smooth_shading;
};

}