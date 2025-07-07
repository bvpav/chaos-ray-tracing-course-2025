#pragma once

#include <span>
#include <utility>
#include <vector>
#include <cassert>

#include "crt_triangle.h"
#include "crt_vector.h"

namespace crt {

struct Mesh {
    Mesh(std::vector<Vector> &&vertices, std::span<const int> indices)
        : vertices(std::move(vertices))
    {
        assert(indices.size() % 3 == 0);
        
        triangles.reserve(indices.size() / 3);
        for (size_t i = 0; i < indices.size(); i += 3) {
            triangles.emplace_back(this->vertices[indices[i]], this->vertices[indices[i + 1]], this->vertices[indices[i + 2]]);
        }
    }

    std::vector<Vector> vertices;
    std::vector<Triangle> triangles;
};

}