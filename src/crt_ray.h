#pragma once

#include <optional>

#include "crt_vector.h"

namespace crt {

struct Ray {
    Vector origin, direction;
    int depth{0};

    Vector at(const float t) const {
        return origin + direction * t;
    }

    /**
     * Generate a reflection ray, originating from a point on the current ray.
     *
     * @warning point MUST be a point on the original ray. Only call it with values, returned by at().
     */
    Ray reflected_at(const Vector &point, const Vector &normal, const float reflection_bias = 1e-2f) const {
        return { point + normal * reflection_bias, direction.reflected(normal), depth + 1 };
    }

    /**
     * Generate a refraction ray, originating from a point on the current ray.
     *
     * @warning point MUST be a point on the original ray. Only call it with values, returned by at().
     */
    std::optional<Ray> refracted_at(const Vector &point, Vector normal, float outside_ior, float inside_ior, const float refraction_bias = 1e-2f) const {
        Ray result = *this;
        if (!result.refract_at(point, normal, outside_ior, inside_ior))
            return std::nullopt;
        return result;
    }

    /**
     * Generate a refraction ray, originating from a point on the current ray.
     *
     * @warning point MUST be a point on the original ray. Only call it with values, returned by at().
     */
    bool refract_at(const Vector &point, Vector normal, float outside_ior, float inside_ior, const float refraction_bias = 1e-2f) {
        if (!direction.refract(normal, outside_ior, inside_ior))
            return false;
        origin = point + -normal * refraction_bias;
        depth++;

        return true;
    }
};

}  