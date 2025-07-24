#pragma once

#include "crt_aabb.h"
#include "crt_acceleration_tree.h"
#include "crt_material.h"
#include "crt_ray.h"
#include "crt_triangle.h"
#include "crt_vector.h"

namespace crt {

struct Intersection {
    float distance;
    Vector point;
    // NOTE: both normals are stored in the intersection record, because smooth
    //       shading is a property of the material and it's used in the shading stage.
    Vector flat_normal, smooth_normal;
    Vector uv;
    float bary_u, bary_v;
    int material_index;

    inline constexpr const Vector &normal(const Material &material) const {
        return material.smooth_shading ? smooth_normal : flat_normal;
    }
};

namespace intersection {

bool ray_intersect_aabb_p(const Ray &ray, const AABB &aabb);
std::optional<Intersection> ray_intersect_triangle(const Ray &ray, const Triangle &triangle);
std::optional<Intersection> ray_intersect_triangle_span(const Ray &ray, std::span<const Triangle> triangles);
std::optional<Intersection> ray_intersect_acceleration_tree(const Ray &ray, const AccelerationTree &meshes);

} // namespace intersection

} // namespace crt