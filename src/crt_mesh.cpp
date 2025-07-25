#include "crt_mesh.h"

#include <cassert>
#include <vector>

namespace crt {

void vertex_array_extend(
    std::vector<Vertex> &vertices, std::vector<Triangle> &triangles,
    std::span<const Vector> positions, std::span<const Vector> uvs, std::span<const int> indices,
    int material_index
)
{
    assert(indices.size() % 3 == 0);
    assert(positions.size() == uvs.size());
    // NOTE: The caller has to have reserved enough memory for all vertices.
    //       This is because the `Triangle` struct is storing references to the vertices inside the std::vector.
    //       Resizing the vector could potentially cause a move, invalidating all existing references.
    assert(vertices.capacity() >= vertices.size() + positions.size());
    
    size_t base_index = vertices.size();

    for (size_t i = 0; i < positions.size(); ++i) {
        vertices.emplace_back(positions[i], Vector {}, uvs[i]);
    }

    // Compute smooth normals of vertices
    for (size_t i = 0; i < indices.size(); i += 3) {
        // HACK: maybe storing references is not a good idea
        Vertex &v0 = vertices[base_index + indices[i]], &v1 = vertices[base_index + indices[i + 1]], &v2 = vertices[base_index + indices[i + 2]];
        triangles.emplace_back(v0, v1, v2, material_index);
        v0.normal += triangles.back().normal();
        v1.normal += triangles.back().normal();
        v2.normal += triangles.back().normal();
    }

    for (Vertex &v : vertices) {
        v.normal.normalize();
    }
}

}