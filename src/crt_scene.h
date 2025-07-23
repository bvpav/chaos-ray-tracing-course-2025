#pragma once

#include <vector>

#include "crt_aabb.h"
#include "crt_camera.h"
#include "crt_image.h"
#include "crt_light.h"
#include "crt_material.h"
#include "crt_mesh.h"
#include "crt_texture.h"

namespace crt {

struct Scene {
    Color background_color;
    Camera camera;
    std::vector<Texture> textures;
    AABB bounds;
    std::vector<Mesh> meshes;
    std::vector<Light> lights;
    std::vector<Material> materials;
    int bucket_size;
};

}