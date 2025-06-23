#pragma once

#include "crt_vector.h"

namespace crt {

/**
 * @brief Triangle is defined by three vertices in counter-clockwise order.
 */
struct Triangle {
    Vector vertices[3];

    Vector normal() const {
        Vector edge1 = vertices[1] - vertices[0];
        Vector edge2 = vertices[2] - vertices[0];
        return edge2.cross(edge1).normalized();
    }
};

}