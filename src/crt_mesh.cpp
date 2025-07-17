#include "crt_mesh.h"

#include "crt_vertex.h"

namespace crt {

    Mesh::Mesh(std::span<const Vector> positions, std::span<const Vector> uvs, std::span<const int> indices, int material_index)
        : material_index(material_index)
    {
        assert(indices.size() % 3 == 0);
        assert(positions.size() == uvs.size());  // Ensure matching sizes
        
        // Create vertices with both position and UV data
        vertices.reserve(positions.size());
        for (size_t i = 0; i < positions.size(); ++i) {
            vertices.emplace_back(positions[i], Vector {}, uvs[i]);
        }

        // Compute smooth normals of vertices
        triangles.reserve(indices.size() / 3);
        for (size_t i = 0; i < indices.size(); i += 3) {
            Vertex &v0 = this->vertices[indices[i]], &v1 = this->vertices[indices[i + 1]], &v2 = this->vertices[indices[i + 2]];
            triangles.emplace_back(v0, v1, v2);
            v0.normal += triangles.back().normal();
            v1.normal += triangles.back().normal();
            v2.normal += triangles.back().normal();
        }

        for (Vertex &v : vertices) {
            v.normal.normalize();
        }
    }
}