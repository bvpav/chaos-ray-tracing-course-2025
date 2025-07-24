#pragma once

#include <vector>

#include "crt_acceleration_tree.h"
#include "crt_camera.h"
#include "crt_image.h"
#include "crt_light.h"
#include "crt_material.h"
#include "crt_texture.h"
#include "crt_vertex.h"

namespace crt {

struct Scene {
    Color background_color;
    Camera camera;
    std::vector<Vertex> vertices;
    AccelerationTree acceleration_tree;
    std::vector<Light> lights;
    std::vector<Texture> textures;
    std::vector<Material> materials;
    int bucket_size;
};

}