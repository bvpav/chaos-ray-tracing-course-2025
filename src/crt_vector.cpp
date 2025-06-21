#include "crt_vector.h"

#include <cmath>

namespace crt {

float Vector::length() const {
    return std::sqrt(length_squared());
}

}