#include <iostream>
#include <fstream>
#include <cstdint>

static constexpr int IMAGE_WIDTH = 1920;
static constexpr int IMAGE_HEIGHT = 1080;

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

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

    for (int row_idx = 0; row_idx < IMAGE_HEIGHT; ++row_idx) {
        for (int col_idx = 0; col_idx < IMAGE_WIDTH; ++col_idx) {
            auto r = double(col_idx) / (IMAGE_WIDTH - 1);
            auto g = double(row_idx) / (IMAGE_HEIGHT - 1);
            auto b = 0.25;

            output_file << static_cast<int>(r * MAX_COLOR_COMPONENT) << ' '
                        << static_cast<int>(g * MAX_COLOR_COMPONENT) << ' '
                        << static_cast<int>(b * MAX_COLOR_COMPONENT) << ' ';
        }
        output_file << '\n';
    }

    output_file.close();
    return 0;
}