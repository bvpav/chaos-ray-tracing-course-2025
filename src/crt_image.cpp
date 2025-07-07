#include "crt_image.h"

#include <algorithm>

namespace crt {

std::ostream &operator<<(std::ostream &os, const Image::PPM &ppm) {
    os << "P3\n"
       << ppm.m_image.width << ' ' << ppm.m_image.height << '\n'
       << ppm.m_max_color_component << '\n';

    for (int raster_y = 0; raster_y < ppm.m_image.height; ++raster_y) {
        for (int raster_x = 0; raster_x < ppm.m_image.width; ++raster_x) {
            crt::Color color = ppm.m_image.pixels[raster_y * ppm.m_image.width + raster_x] * ppm.m_max_color_component;
            os << std::clamp(static_cast<int>(color.x), 0, ppm.m_max_color_component) << ' '
               << std::clamp(static_cast<int>(color.y), 0, ppm.m_max_color_component) << ' '
               << std::clamp(static_cast<int>(color.z), 0, ppm.m_max_color_component) << '\t';
        }
        os << '\n';
    }

    return os;
}

}