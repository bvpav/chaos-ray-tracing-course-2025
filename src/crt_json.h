#pragma once

#include <istream>
#include <optional>

#include "crt_scene.h"

namespace crt::json {

std::optional<Scene> get_scene_from_istream(std::istream &is);

}