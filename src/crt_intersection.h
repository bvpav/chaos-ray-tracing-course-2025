#pragma once

#include "crt_material.h"
#include "crt_mesh.h"
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

std::optional<Intersection> ray_intersect_triangle(const Ray &ray, const Triangle &triangle);
std::optional<Intersection> ray_intersect_mesh_span(const Ray &ray, std::span<const Mesh> meshes);

} // namespace intersection

} // namespace crt