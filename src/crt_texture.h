#pragma once

#include "crt_image.h"
#include "crt_vector.h"

namespace crt {
    
enum class TextureType {
    Albedo,
    Edges,
    Checker,
    Bitmap,
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

struct BitmapTexture {
    // FIXME: Level of inderection added because Image is not trivially distructable and cannot be part of the enum added because Image is not trivially distructable and cannot be part of the enum
    //        I think I should use std::variant for this...
    Image *image;
};

struct Texture {
    TextureType type;

    union {
        AlbedoTexture as_albedo_tex;
        EdgesTexture as_edges_tex;
        CheckerTexture as_checker_tex;
        BitmapTexture as_bitmap_tex;
    };

    const Color &sample(const Vector &uv, float bary_u, float bary_v) const;

    // ~Texture() {
    //     if (type == TextureType::Bitmap)
    //         delete as_bitmap_tex.image;
    // }
};
    
}