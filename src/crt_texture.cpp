#include "crt_texture.h"
#include <cassert>

namespace crt {

const Color &Texture::sample(float u, float v) const {
    switch (type) {
        case TextureType::Albedo:
            return as_albedo_tex.albedo;
            break;
    }

    // std::unreachable() // FIXME: Use C++23 maybe?
    assert(false);
}

}