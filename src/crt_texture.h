#pragma once

#include "crt_image.h"

namespace crt {
    
enum class TextureType {
    Albedo,
    Edges,
};

struct AlbedoTexture {
    Color albedo;
};
    
struct EdgesTexture {
    Color edge_color, inner_color;
    float edge_width;
};
    
struct Texture {
    TextureType type;

    union {
        AlbedoTexture as_albedo_tex;
        EdgesTexture as_edges_tex;
    };

    const Color &sample(float u, float v, float bary_u, float bary_v) const;
};
    
}