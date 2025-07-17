#pragma once

#include <cstdint>

#include "crt_image.h"
#include "crt_scene.h"

namespace crt {

struct RendererSettings {
    uint32_t max_ray_depth;
    float shadow_bias;
    float reflection_bias;
    float refraction_bias;
};

Image render_image(const Scene &scene, const RendererSettings &settings);

}