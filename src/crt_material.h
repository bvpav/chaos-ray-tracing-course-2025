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
    std::string albedo_map_texture_name;
    float ior;
    bool smooth_shading;
};

}