#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <cmath>

static constexpr int IMAGE_WIDTH = 887;
static constexpr int IMAGE_HEIGHT = 665;

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

static constexpr int CIRCLE_RADIUS = 150;

int main(int argc, char *argv[]) {
    auto output_path = argc > 1 ? argv[1] : "output.ppm";
    std::ofstream output_file(output_path, std::ios::out | std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open file: " << output_path << '\n';
        return 1;
    }

    output_file << "P3\n"
                << IMAGE_WIDTH << ' ' << IMAGE_HEIGHT << '\n'
                << MAX_COLOR_COMPONENT << '\n';

    for (int screen_y = 0; screen_y < IMAGE_HEIGHT; ++screen_y) {
        for (int screen_x = 0; screen_x < IMAGE_WIDTH; ++screen_x) {
            float distance_x = screen_x - IMAGE_WIDTH / 2.0f;
            float distance_y = screen_y - IMAGE_HEIGHT / 2.0f;

            float distance_squared = distance_x * distance_x + distance_y * distance_y;

            if (distance_squared <= CIRCLE_RADIUS * CIRCLE_RADIUS) {
                output_file << "58 118 25 ";
            } else {
                output_file << "183 183 183 ";
            }
        }
        output_file << '\n';
    }

    output_file.close();
    return 0;
}