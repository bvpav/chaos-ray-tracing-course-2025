#pragma once

#include <span>

#include "crt_aabb.h"
#include "crt_acceleration_tree.h"
#include "crt_ray.h"
#include "crt_triangle.h"
#include "crt_vector.h"

namespace crt {

struct Intersection {
    float distance;
    Vector point;
    Vector normal;
    Vector uv;
    float bary_u, bary_v;
    int material_index;
};

namespace intersection {

bool ray_intersect_aabb_p(const Ray &ray, const AABB &aabb);
std::optional<Intersection> ray_intersect_triangle(const Ray &ray, const Triangle &triangle);
std::optional<Intersection> ray_intersect_triangle_span(const Ray &ray, std::span<const Triangle> triangles);
std::optional<Intersection> ray_intersect_acceleration_tree(const Ray &ray, const AccelerationTree &acceleration_tree);

} // namespace intersection

} // namespace crt