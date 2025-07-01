#pragma once

#include <array>

#include "crt_vector.h"

namespace crt {

/**
 * @brief Triangle is defined by three vertices in counter-clockwise order.
 */
class Triangle {
public:
    Triangle(const Vector &v0, const Vector &v1, const Vector &v2)
        : m_vertices{ v0, v1, v2 }
    {
        Vector edge0 = m_vertices[1] - m_vertices[0];
        Vector edge1 = m_vertices[2] - m_vertices[0];
        m_normal = edge0.cross(edge1).normalized();
    }

    Vector normal() const {
        return m_normal;
    }

    float area() const {
        Vector edge0 = m_vertices[1] - m_vertices[0];
        Vector edge1 = m_vertices[2] - m_vertices[0];
        return edge0.cross(edge1).length() * 0.5;
    }

    const std::array<Vector, 3> &vertices() const {
        return m_vertices;
    }

    std::array<Vector, 3> edges() const {
        return {
            m_vertices[1] - m_vertices[0],
            m_vertices[2] - m_vertices[1],
            m_vertices[0] - m_vertices[2]
        };
    }

private:
    std::array<Vector, 3> m_vertices;
    Vector m_normal;
};

}