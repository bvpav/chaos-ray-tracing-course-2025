#include "crt_texture.h"

#include <cassert>
#include <cmath>
#include <utility>

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

            // NOTE: This can be optimized further by storing the recirpical of square_size.
            //       It's currently not done to avoid premature optimization...
            int row = static_cast<int>(uv.x / ct.square_size);
            int col = static_cast<int>(uv.y / ct.square_size);

            if ((row + col) & 1)
                return ct.color_b;
            else
                return ct.color_a;
        }

        case TextureType::Bitmap: {
            const Image &bti = *as_bitmap_tex.image;

            int raster_x = static_cast<int>(uv.x * bti.width) % bti.width;
            int raster_y = static_cast<int>((1.0f - uv.y) * bti.height) % bti.height;

            return bti.buffer[raster_y * bti.width + raster_x];
        }
    }

    std::unreachable();
}

}