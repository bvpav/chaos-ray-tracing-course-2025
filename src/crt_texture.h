#pragma once

#include "crt_image.h"
#include "crt_vector.h"

namespace crt {
    
enum class TextureType {
    Albedo,
    Edges,
    Checker,
};

struct AlbedoTexture {
    Color albedo;
};
    
struct EdgesTexture {
    Color edge_color, inner_color;
    float edge_width;
};

struct CheckerTexture {
    Color color_a, color_b;
    float square_size;
};

struct Texture {
    TextureType type;

    union {
        AlbedoTexture as_albedo_tex;
        EdgesTexture as_edges_tex;
        CheckerTexture as_checker_tex;
    };

    const Color &sample(const Vector &uv, float bary_u, float bary_v) const;
};
    
}