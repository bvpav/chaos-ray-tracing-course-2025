#include "crt_texture.h"
#include <cassert>

namespace crt {

const Color &Texture::sample(const Vector &uv, float bary_u, float bary_v) const {
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

        case TextureType::Checker: {
            const CheckerTexture &ct = as_checker_tex;

            float u, v;
            for (u = uv.x; u > 2.0f * ct.square_size; u -= 2.0f * ct.square_size);
            for (v = uv.y; v > 2.0f * ct.square_size; v -= 2.0f * ct.square_size);

            if (u <= ct.square_size && v <= ct.square_size
                    || u >= ct.square_size && v >= ct.square_size)
                return ct.color_a;
            else
                return ct.color_b;
        }
    }

    // std::unreachable() // FIXME: Use C++23 maybe?
    assert(false);
}

}