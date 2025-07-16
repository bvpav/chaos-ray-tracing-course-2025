#pragma once

#include <filesystem>
#include <optional>

#include "crt_image.h"

namespace crt {

std::optional<Image> read_stb(const std::filesystem::path &filename);

}