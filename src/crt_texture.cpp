#include "crt_texture.h"
#include <cassert>

namespace crt {

const Color &Texture::sample(Vector uv, float bary_u, float bary_v) const {
    switch (type) {
        case TextureType::Albedo:
            return as_albedo_tex.albedo;
        
        case TextureType::Edges: {
            const EdgesTexture &et = as_edges_tex;
            if (bary_u <= et.edge_width
                    || bary_v <= et.edge_width
                    || (1.0f - bary_u - bary_v) <= et.edge_width)
                return et.edge_color;
            else 
                return et.inner_color;
        }
    }

    // std::unreachable() // FIXME: Use C++23 maybe?
    assert(false);
}

}