#pragma once

#include <tuple>

#include "crt_vertex.h"
#include "crt_vector.h"

namespace crt {

/**
 * @brief Triangle is defined by three vertices in counter-clockwise order.
 */
class Triangle {
public:
    Triangle(const Vertex &v0, const Vertex &v1, const Vertex &v2, int material_index)
        : m_v0(v0), m_v1(v1), m_v2(v2)
        , material_index(material_index)
    {
        Vector edge0 = m_v1.position - m_v0.position;
        Vector edge1 = m_v2.position - m_v0.position;
        m_normal = edge0.cross(edge1).normalized();
    }

    Vector normal() const {
        return m_normal;
    }

    float area() const {
        Vector edge0 = m_v1.position - m_v0.position;
        Vector edge1 = m_v2.position - m_v0.position;
        return edge0.cross(edge1).length() * 0.5;
    }

    std::tuple<const Vertex &, const Vertex &, const Vertex &> vertices() const {
        return { m_v0, m_v1, m_v2 };
    }

    std::tuple<Vector, Vector, Vector> edges() const {
        return {
            m_v1.position - m_v0.position,
            m_v2.position - m_v1.position,
            m_v0.position - m_v2.position
        };
    }

    int material_index;

private:
    const Vertex &m_v0, &m_v1, &m_v2;
    Vector m_normal;
};

}