#pragma once

#include "crt_image.h"

namespace crt {
    
enum class TextureType {
    Albedo,
};

struct AlbedoTexture {
    Color albedo;
};
    
struct Texture {
    TextureType type;

    union {
        AlbedoTexture as_albedo_tex;
    };

    const Color &sample(float u, float v) const;
};
    
}