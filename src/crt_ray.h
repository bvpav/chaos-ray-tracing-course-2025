#pragma once

#include "crt_vector.h"

namespace crt {

struct Ray {
    Vector origin, direction;

    Vector at(float t) const {
        return origin + direction * t;
    }
};

}  