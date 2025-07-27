#pragma once

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
};

}