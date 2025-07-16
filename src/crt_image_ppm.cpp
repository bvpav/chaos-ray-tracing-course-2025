#include "crt_image_ppm.h"
#include "crt_image.h"

#include <algorithm>

namespace crt {

void write_ppm(const Image &image, std::ostream &os, int max_color_component) {
    os << "P3\n"
       << image.width << ' ' << image.height << '\n'
       << max_color_component << '\n';

    for (int raster_y = 0; raster_y < image.height; ++raster_y) {
        for (int raster_x = 0; raster_x < image.width; ++raster_x) {
            crt::Color color = image.pixels[raster_y * image.width + raster_x] * max_color_component;
            os << std::clamp(static_cast<int>(color.x), 0, max_color_component) << ' '
               << std::clamp(static_cast<int>(color.y), 0, max_color_component) << ' '
               << std::clamp(static_cast<int>(color.z), 0, max_color_component) << '\t';
        }
        os << '\n';
    }
}

}