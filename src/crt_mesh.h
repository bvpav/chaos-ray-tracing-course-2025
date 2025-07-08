#pragma once

#include <span>
#include <vector>
#include <cassert>

#include "crt_triangle.h"
#include "crt_vector.h"
#include "crt_vertex.h"

namespace crt {

struct Mesh {
    Mesh(std::span<const Vector> positions, std::span<const int> indices, int material_index);

    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    int material_index;
};

}