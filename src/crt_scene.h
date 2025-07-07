#pragma once

#include <vector>

#include "crt_camera.h"
#include "crt_image.h"
#include "crt_light.h"
#include "crt_mesh.h"

namespace crt {

struct Scene {
    Color background_color;
    Camera camera;
    std::vector<Mesh> meshes;
    std::vector<Light> lights;
};

}