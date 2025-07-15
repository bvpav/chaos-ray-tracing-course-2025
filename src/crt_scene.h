#pragma once

#include <string>
#include <unordered_map>
#include <vector>

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
    std::unordered_map<std::string, Texture> textures;
    std::vector<Mesh> meshes;
    std::vector<Light> lights;
    std::vector<Material> materials;
};

}