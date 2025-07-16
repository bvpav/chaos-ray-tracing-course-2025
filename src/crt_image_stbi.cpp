#include "crt_image_stbi.h"
#include "crt_vector.h"

#include <bit>
#include <experimental/scope>
#include <optional>
#include <span>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using std::experimental::scope_exit;

namespace crt {

std::optional<Image> read_stb(const std::filesystem::path &filename) {
    int width, height, num_components;
    float *buffer = stbi_loadf(filename.c_str(), &width, &height, &num_components, STBI_rgb);
    if (!buffer)
        return std::nullopt;

    scope_exit guard{ [&](){ stbi_image_free(buffer); } };

    if (num_components != 3)
        return std::nullopt;

    std::span<const Vector> pixels{
        std::bit_cast<const Vector *>(buffer),
        static_cast<size_t>(width * height)
    };

    return Image { width, height, pixels };
}

}