#pragma once

#include "crt_vector.h"

namespace crt {

/**
 * An infinitely small point/omnidirectional light source.
 */
struct Light {
    float intensity;
    /**
     * World space position of the light
     */
    Vector position;
};

}