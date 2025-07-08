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
    Triangle(const Vertex &v0, const Vertex &v1, const Vertex &v2)
        : m_v0(v0), m_v1(v1), m_v2(v2)
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

    std::tuple<const crt::Vertex &, const crt::Vertex &, const crt::Vertex &> vertices() const {
        return { m_v0, m_v1, m_v2 };
    }

    std::tuple<crt::Vector, crt::Vector, crt::Vector> edges() const {
        return {
            m_v1.position - m_v0.position,
            m_v2.position - m_v1.position,
            m_v0.position - m_v2.position
        };
    }

private:
    const Vertex &m_v0, &m_v1, &m_v2;
    Vector m_normal;
};

}