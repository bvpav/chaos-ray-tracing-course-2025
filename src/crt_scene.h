#pragma once

#include <cstdint>
#include <vector>

#include "crt_acceleration_tree.h"
#include "crt_camera.h"
#include "crt_image.h"
#include "crt_light.h"
#include "crt_material.h"
#include "crt_texture.h"
#include "crt_vertex.h"

namespace crt {

inline constexpr int DEFAULT_SCENE_BUCKET_SIZE = 24;

struct Scene {
    Color background_color;
    Camera camera;
    std::vector<Vertex> vertices;
    AccelerationTree acceleration_tree;
    std::vector<Light> lights;
    std::vector<Texture> textures;
    std::vector<Material> materials;
    int bucket_size;
    uint8_t gi_on : 1;
    uint8_t reflections_on : 1 {1};
    uint8_t refractions_on : 1 {1};
};

}