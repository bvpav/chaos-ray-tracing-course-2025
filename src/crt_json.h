#pragma once

#include <filesystem>
#include <istream>
#include <optional>

#include "crt_scene.h"

namespace crt::json {

std::optional<Scene> read_scene_from_istream(std::istream &is, const std::filesystem::path &asset_root);

}