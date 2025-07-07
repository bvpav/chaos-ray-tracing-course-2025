#pragma once

#include <span>
#include <vector>
#include <cassert>

#include "crt_triangle.h"
#include "crt_vector.h"

namespace crt {

class Mesh {
public:
    // FIXME: Unnecessary copy of vertices done here, will be fixed once the internal representation of Mesh changes
    Mesh(std::span<const Vector> vertices, std::span<const int> indices)
        : m_vertices(vertices.begin(), vertices.end())
    {
        assert(indices.size() % 3 == 0);
        
        m_triangles.reserve(indices.size() / 3);
        for (size_t i = 0; i < indices.size(); i += 3) {
            m_triangles.emplace_back(m_vertices[indices[i]], m_vertices[indices[i + 1]], m_vertices[indices[i + 2]]);
        }
    }

    std::span<const Triangle> triangles() const {
        return m_triangles;
    }

private:
    std::vector<Vector> m_vertices;
    std::vector<Triangle> m_triangles;
};

}