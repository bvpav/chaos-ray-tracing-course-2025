#include <experimental/scope>
#include <optional>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "crt_image_stbi.h"
#include "crt_image.h"

using std::experimental::scope_exit;

namespace crt {

std::optional<Image> read_stb(const std::filesystem::path &filename) {
    int width, height, num_components;
    uint8_t *buffer = stbi_load(filename.c_str(), &width, &height, &num_components, STBI_rgb);
    if (!buffer)
        return std::nullopt;

    scope_exit guard{ [&](){ stbi_image_free(buffer); } };

    if (num_components != 3)
        return std::nullopt;

    std::vector<Color> pixels;
    pixels.reserve(width * height);
    for (size_t i = 0; i < width * height * 3; i += 3) {
        pixels.emplace_back(
            Color {
                buffer[i] / 255.0f,
                buffer[i + 1] / 255.0f,
                buffer[i + 2] / 255.0f
            }
        );
    }

    return Image { width, height, std::move(pixels) };
}

}