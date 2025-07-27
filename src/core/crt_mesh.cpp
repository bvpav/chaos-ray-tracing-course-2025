#include "crt_mesh.h"
#include "crt_triangle.h"

#include <cassert>
#include <span>
#include <vector>

namespace crt {

static void fill_triangles(
    std::vector<Vertex> &vertices, std::vector<Triangle> &triangles,
    std::span<const int> indices,
    int base_index, int material_index,
    TriangleFlags flags
) noexcept {
    assert(indices.size() % 3 == 0);

    // Compute smooth normals of vertices
    for (size_t i = 0; i < indices.size(); i += 3) {
        Vertex &v0 = vertices[base_index + indices[i]], &v1 = vertices[base_index + indices[i + 1]], &v2 = vertices[base_index + indices[i + 2]];
        triangles.emplace_back(&v0, &v1, &v2, material_index, flags);
        v0.normal += triangles.back().face_normal;
        v1.normal += triangles.back().face_normal;
        v2.normal += triangles.back().face_normal;
    }

    for (Vertex &v : vertices) {
        v.normal.normalize();
    }
}

void vertex_array_extend(
    std::vector<Vertex> &vertices, std::vector<Triangle> &triangles,
    std::span<const Vector> positions, std::span<const Vector> uvs, std::span<const int> indices,
    int material_index,
    TriangleFlags triangle_flags
)
{
    assert(positions.size() == uvs.size());
    // NOTE: The caller has to have reserved enough memory for all vertices.
    //       This is because the `Triangle` struct is storing raw pointers to the vertices inside the std::vector.
    //       Resizing the vector could potentially cause a move, invalidating all existing pointers.
    assert(vertices.capacity() >= vertices.size() + positions.size());
    
    size_t base_index = vertices.size();

    for (size_t i = 0; i < positions.size(); ++i) {
        vertices.emplace_back(positions[i], Vector {}, uvs[i]);
    }

    fill_triangles(vertices, triangles, indices, base_index, material_index, triangle_flags);
}

void vertex_array_extend(
    std::vector<Vertex> &vertices, std::vector<Triangle> &triangles,
    std::span<const Vector> positions, std::span<const int> indices,
    int material_index,
    TriangleFlags triangle_flags
)
{
    // NOTE: The caller has to have reserved enough memory for all vertices.
    //       This is because the `Triangle` struct is storing raw pointers to the vertices inside the std::vector.
    //       Resizing the vector could potentially cause a move, invalidating all existing pointers.
    assert(vertices.capacity() >= vertices.size() + positions.size());
    
    size_t base_index = vertices.size();

    for (size_t i = 0; i < positions.size(); ++i) {
        vertices.emplace_back(positions[i], Vector {}, Vector {});
    }

    fill_triangles(vertices, triangles, indices, base_index, material_index, triangle_flags);
}

}