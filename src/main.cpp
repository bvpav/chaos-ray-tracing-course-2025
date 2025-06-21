#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>

static constexpr int IMAGE_WIDTH = 800;
static constexpr int IMAGE_HEIGHT = 600;

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

static constexpr int RECTANGLE_COUNT = 4;
static constexpr int NOISE_MIN = -MAX_COLOR_COMPONENT;
static constexpr int NOISE_MAX = MAX_COLOR_COMPONENT / 3;

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
            int rect_h_index = screen_y / (IMAGE_HEIGHT / RECTANGLE_COUNT);
            int rect_w_index = screen_x / (IMAGE_WIDTH / RECTANGLE_COUNT);
            int rect_index = rect_h_index * RECTANGLE_COUNT + rect_w_index;

            // Resolve an adjacently unique color for each rectangle
            int color_index = rect_index % 6 + 1;
            int r = (color_index & 1) ? MAX_COLOR_COMPONENT : 0;
            int g = (color_index & 2) ? MAX_COLOR_COMPONENT : 0;
            int b = (color_index & 4) ? MAX_COLOR_COMPONENT : 0;

            int noise_r = rand() % (NOISE_MAX - NOISE_MIN + 1) + NOISE_MIN;
            int noise_g = rand() % (NOISE_MAX - NOISE_MIN + 1) + NOISE_MIN;
            int noise_b = rand() % (NOISE_MAX - NOISE_MIN + 1) + NOISE_MIN;

            r = std::clamp(r + noise_r, 0, MAX_COLOR_COMPONENT);
            g = std::clamp(g + noise_g, 0, MAX_COLOR_COMPONENT);
            b = std::clamp(b + noise_b, 0, MAX_COLOR_COMPONENT);

            output_file << r << ' ' << g << ' ' << b << '\n';
        }
        output_file << '\n';
    }

    output_file.close();
    return 0;
}