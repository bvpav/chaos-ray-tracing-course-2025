#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <iomanip>

#include "crt_ray.h"

static constexpr int LARGE_RESOLUTION_X = 1920;
static constexpr int LARGE_RESOLUTION_Y = 1920;

static constexpr int START_RESOLUTION_X = 1920;
static constexpr int START_RESOLUTION_Y = 1080;

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

int main(int argc, char *argv[]) {
    float theta = 0.0f;
    for (int frame_index = 0; frame_index < 60; ++frame_index) {
        std::stringstream output_path;
        output_path << "output_" << std::setfill('0') << std::setw(2) << frame_index << ".ppm";
        std::ofstream output_file(output_path.str(), std::ios::out | std::ios::binary);
        if (!output_file.is_open()) {
            std::cerr << "Error: Could not open file: " << output_path.str() << '\n';
            return 1;
        }

        int resolution_x = START_RESOLUTION_X * std::abs(std::cos(theta)) + START_RESOLUTION_Y * std::abs(std::sin(theta));
        int resolution_y = START_RESOLUTION_X * std::abs(std::sin(theta)) + START_RESOLUTION_Y * std::abs(std::cos(theta));

        output_file << "P3\n"
                    << LARGE_RESOLUTION_X << ' ' << LARGE_RESOLUTION_Y << '\n'
                    << MAX_COLOR_COMPONENT << '\n';

        for (int big_raster_y = 0; big_raster_y < LARGE_RESOLUTION_Y; ++big_raster_y) {
            for (int big_raster_x = 0; big_raster_x < LARGE_RESOLUTION_X; ++big_raster_x) {
                if (big_raster_x >= LARGE_RESOLUTION_X / 2 - resolution_x / 2 && big_raster_x < LARGE_RESOLUTION_X / 2 + resolution_x / 2 &&
                    big_raster_y >= LARGE_RESOLUTION_Y / 2 - resolution_y / 2 && big_raster_y < LARGE_RESOLUTION_Y / 2 + resolution_y / 2) {
                    int raster_x = big_raster_x - (LARGE_RESOLUTION_X / 2 - resolution_x / 2);
                    int raster_y = big_raster_y - (LARGE_RESOLUTION_Y / 2 - resolution_y / 2);

                    crt::Ray ray;

                    // Pixel center in raster space
                    ray.direction = { raster_x + 0.5f, raster_y + 0.5f, 0.0f };

                    // Raster space to NDC space
                    ray.direction.x /= resolution_x;
                    ray.direction.y /= resolution_y;

                    // NDC space to screen space
                    ray.direction.x = (2.0f * ray.direction.x) - 1.0f;
                    ray.direction.y = 1.0f - (2.0f * ray.direction.y);

                    // Consider aspect ratio
                    ray.direction.x *= float(resolution_x) / resolution_y;

                    ray.direction.z = -1.0f;
                    ray.direction = ray.direction.normalized();

                    ray.origin = { 0.0f, 0.0f, 0.0f };

                    crt::Vector color = (ray.direction * 0.5f + 0.5f) * MAX_COLOR_COMPONENT;
                    output_file << static_cast<int>(color.x) << ' '
                                << static_cast<int>(color.y) << ' '
                                << static_cast<int>(color.z) << ' ';
                } else {
                    output_file << "0 0 0 ";
                }
            }
            output_file << '\n';
        }

        output_file.close();

        theta += 0.05235988;
    }
    return 0;
}