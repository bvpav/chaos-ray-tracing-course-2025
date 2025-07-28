#pragma once

#include <cstdint>

#include "crt_image.h"
#include "crt_scene.h"

namespace crt {

inline constexpr int DEFAULT_MAX_RAY_DEPTH = 3;
inline constexpr int DEFAULT_DIFFUSE_REFLECTION_RAY_COUNT = 4;

inline constexpr float DEFAULT_SHADOW_BIAS = 1e-2f;
inline constexpr float DEFAULT_REFLECTION_BIAS = 1e-2f;
inline constexpr float DEFAULT_DIFFUSE_REFLECTION_BIAS = 1e-2f;
inline constexpr float DEFAULT_REFRACTION_BIAS = 1e-2f;

struct RendererSettings {
    uint32_t max_ray_depth{ DEFAULT_MAX_RAY_DEPTH };
    uint32_t diffuse_reflection_ray_count{ DEFAULT_DIFFUSE_REFLECTION_RAY_COUNT };
    float shadow_bias{ DEFAULT_SHADOW_BIAS };
    float reflection_bias{ DEFAULT_REFLECTION_BIAS };
    float diffuse_reflection_bias{ DEFAULT_DIFFUSE_REFLECTION_BIAS };
    float refraction_bias{ DEFAULT_REFRACTION_BIAS };
};

Image render_image(const Scene &scene, const RendererSettings &settings);

}