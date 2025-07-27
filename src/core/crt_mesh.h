#pragma once

#include <span>
#include <vector>
#include <cassert>

#include "crt_triangle.h"
#include "crt_vector.h"
#include "crt_vertex.h"

namespace crt {

void vertex_array_extend(
    std::vector<Vertex> &vertices, std::vector<Triangle> &triangles,
    std::span<const Vector> positions, std::span<const Vector> uvs, std::span<const int> indices,
    int material_index,
    TriangleFlags triangle_flags
);

void vertex_array_extend(
    std::vector<Vertex> &vertices, std::vector<Triangle> &triangles,
    std::span<const Vector> positions, std::span<const int> indices,
    int material_index,
    TriangleFlags triangle_flags
);

}