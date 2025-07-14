#pragma once

#include "crt_image.h"

namespace crt {

enum class MaterialType {
    Diffuse,
    Reflective,
    Refractive,
};

struct Material {
    MaterialType type;
    Color albedo;
    float ior;
    bool smooth_shading;
};

}