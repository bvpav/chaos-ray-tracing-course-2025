#pragma once

#include <array>
#include <tuple>
#include <vector>

#include "crt_vector.h"

namespace crt {

/**
 * @brief Triangle is defined by three vertices in counter-clockwise order.
 */
class Triangle {
public:
    Triangle(const Vector &v0, const Vector &v1, const Vector &v2)
        : m_v0(v0), m_v1(v1), m_v2(v2)
    {
        Vector edge0 = m_v1 - m_v0;
        Vector edge1 = m_v2 - m_v0;
        m_normal = edge0.cross(edge1).normalized();
    }

    Vector normal() const {
        return m_normal;
    }

    float area() const {
        Vector edge0 = m_v1 - m_v0;
        Vector edge1 = m_v2 - m_v0;
        return edge0.cross(edge1).length() * 0.5;
    }

    std::tuple<const crt::Vector &, const crt::Vector &, const crt::Vector &> vertices() const {
        return { m_v0, m_v1, m_v2 };
    }

    std::tuple<crt::Vector, crt::Vector, crt::Vector> edges() const {
        return {
            m_v1 - m_v0,
            m_v2 - m_v1,
            m_v0 - m_v2
        };
    }

private:
    const Vector &m_v0, &m_v1, &m_v2;
    Vector m_normal;
};

}