#pragma once

#include "crt_image.h"

namespace crt {

enum class MaterialType {
    Diffuse,
    Reflective,
};

struct Material {
    MaterialType type;
    Color albedo;
    bool smooth_shading;
};

}