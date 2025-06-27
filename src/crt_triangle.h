#pragma once

#include "crt_vector.h"

namespace crt {

/**
 * @brief Triangle is defined by three vertices in counter-clockwise order.
 */
class Triangle {
public:
    Triangle(const Vector& v0, const Vector& v1, const Vector& v2)
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

private:
    Vector m_vertices[3];
    Vector m_normal;
};

}