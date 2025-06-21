#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>

#include "crt_ray.h"

static constexpr int RESOLUTION_X = 1920;
static constexpr int RESOLUTION_Y = 1080;

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

int main(int argc, char *argv[]) {
    auto output_path = argc > 1 ? argv[1] : "output.ppm";
    std::ofstream output_file(output_path, std::ios::out | std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open file: " << output_path << '\n';
        return 1;
    }

    output_file << "P3\n"
                << RESOLUTION_X << ' ' << RESOLUTION_Y << '\n'
                << MAX_COLOR_COMPONENT << '\n';

    for (int raster_y = 0; raster_y < RESOLUTION_Y; ++raster_y) {
        for (int raster_x = 0; raster_x < RESOLUTION_X; ++raster_x) {
            crt::Ray ray;

            // Pixel center in raster space
            ray.direction = { raster_x + 0.5f, raster_y + 0.5f, 0.0f };

            // Raster space to NDC space
            ray.direction.x /= RESOLUTION_X;
            ray.direction.y /= RESOLUTION_Y;

            // NDC space to screen space
            ray.direction.x = (2.0f * ray.direction.x) - 1.0f;
            ray.direction.y = 1.0f - (2.0f * ray.direction.y);

            // Consider aspect ratio
            ray.direction.x *= float(RESOLUTION_X) / RESOLUTION_Y;

            ray.direction.z = -1.0f;
            ray.direction = ray.direction.normalized();

            ray.origin = { 0.0f, 0.0f, 0.0f };

            crt::Vector color = (ray.direction * 0.5f + 0.5f) * MAX_COLOR_COMPONENT;
            output_file << static_cast<int>(color.x) << ' '
                        << static_cast<int>(color.y) << ' '
                        << static_cast<int>(color.z) << ' ';
        }
        output_file << '\n';
    }

    output_file.close();
    return 0;
}