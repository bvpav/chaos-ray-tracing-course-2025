#pragma once

#include <cassert>
#include <limits>
#include <utility>

#include "crt_vector.h"

namespace crt {

struct AABB {
    Vector min, max;

    /**
     * Get an AABB that contains no points in the space.
     */
    static constexpr AABB vacuum() noexcept {
        return {
            {  std::numeric_limits<float>::infinity(),  std::numeric_limits<float>::infinity(),  std::numeric_limits<float>::infinity() },
            { -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() }
        };
    }

    constexpr std::pair<AABB, AABB> split(const unsigned axis) const noexcept {
        assert(axis < 3);

        AABB left  = *this;
        AABB right = *this;

        const float mid = (min.data[axis] + max.data[axis]) * 0.5f;
        left.max.data[axis]  = mid;
        right.min.data[axis] = mid;

        return { left, right };
    }

    constexpr bool intersects(const AABB &other) const noexcept {
        for (int axis = 0; axis < 3; ++axis) {
            if (other.min.data[axis] > max.data[axis])
                return false;
            if (other.max.data[axis] < min.data[axis])
                return false;
        }
        return true;
    }
};

}