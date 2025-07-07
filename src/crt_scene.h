#pragma once

#include <optional>
#include <vector>

#include "crt_camera.h"
#include "crt_image.h"
#include "crt_mesh.h"

namespace crt {

struct Scene {
    Color background_color;
    std::vector<Mesh> meshes;
    Camera camera;
};

}