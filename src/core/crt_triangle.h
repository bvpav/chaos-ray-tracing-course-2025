#pragma once

#include <cstdint>
#include <tuple>

#include "crt_vertex.h"
#include "crt_vector.h"

namespace crt {

struct TriangleFlags {
    uint8_t smooth_shading    : 1;
    uint8_t back_face_culling : 1;
};

/**
 * @brief Triangle is defined by three vertices in counter-clockwise order.
 */
struct Triangle {
    const Vertex *v0, *v1, *v2;
    Vector face_normal;
    int material_index;
    TriangleFlags flags;

    Triangle(const Vertex *v0, const Vertex *v1, const Vertex *v2, int material_index, TriangleFlags flags)
        : v0(v0), v1(v1), v2(v2)
        , material_index(material_index)
        , flags(flags)
    {
        Vector edge0 = v1->position - v0->position;
        Vector edge1 = v2->position - v0->position;
        face_normal = edge0.cross(edge1).normalized();
    }

    constexpr std::tuple<const Vertex &, const Vertex &, const Vertex &> vertices() const {
        return { *v0, *v1, *v2 };
    }

    constexpr std::tuple<Vector, Vector, Vector> edges() const {
        return {
            v1->position - v0->position,
            v2->position - v1->position,
            v0->position - v2->position
        };
    }
};

}