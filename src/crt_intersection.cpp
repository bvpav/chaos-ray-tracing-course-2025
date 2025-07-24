#include "crt_intersection.h"

#include "crt_ray.h"
#include "crt_triangle.h"
#include "crt_vector.h"

namespace crt::intersection {

bool ray_intersect_aabb_p(const Ray &ray, const AABB &aabb) {
    const Vector *extents = reinterpret_cast<const Vector *>(&aabb);

    for (int extent = 0; extent < 2; ++extent) {
        for (int side_axis = 0; side_axis < 3; ++side_axis) {
            bool is_parallel_to_axis = std::abs(ray.direction.data[side_axis]) < 1e-6f;
            if (is_parallel_to_axis) {
                continue;
            }

            const float t = (extents[extent].data[side_axis] - ray.origin.data[side_axis]) / ray.direction.data[side_axis];
            if (t < 0.0f) {
                continue;
            }

            const Vector p = ray.at(t);

            // Check containment in the other 2 axes (u, v)
            constexpr int u_table[] = { 1, 2, 0 };
            constexpr int v_table[] = { 2, 0, 1 };
            const int u = u_table[side_axis], v = v_table[side_axis];

            if (p.data[u] >= aabb.min.data[u] &&
                    p.data[u] <= aabb.max.data[u] &&
                    p.data[v] >= aabb.min.data[v] &&
                    p.data[v] <= aabb.max.data[v]) {
                return true;
            }
        }
    }
    return false;
}

std::optional<Intersection> ray_intersect_triangle(const Ray &ray, const Triangle &triangle) {
    const auto &[v0, v1, v2] = triangle.vertices();
    const auto [e0, e1, e2] = triangle.edges();

    float ray_normal_dist = triangle.normal().dot(ray.direction);
    bool is_parallel_to_plane = std::abs(ray_normal_dist) < 1e-6f;
    if (is_parallel_to_plane) {
        return std::nullopt;
    }

    float origin_plane_dist = triangle.normal().dot(v0.position - ray.origin);
    float intersection_distance = origin_plane_dist / ray_normal_dist;
    if (intersection_distance < 0.0f) {
        return std::nullopt;
    }

    Vector intersection_point = ray.at(intersection_distance);
    Vector v0p = intersection_point - v0.position, v1p = intersection_point - v1.position, v2p = intersection_point - v2.position;
    if (triangle.normal().dot(e0.cross(v0p)) >= 0.0f
            && triangle.normal().dot(e1.cross(v1p)) >= 0.0f
            && triangle.normal().dot(e2.cross(v2p)) >= 0.0f)
    {
        const Vector &v0v1 = e0;
        Vector v0v2 = -e2;
        float bary_u = v0p.cross(v0v2).length() / v0v1.cross(v0v2).length();
        float bary_v = v0v1.cross(v0p).length() / v0v1.cross(v0v2).length();

        const Vector flat_normal = triangle.normal();
        const Vector smooth_normal = v1.normal * bary_u + v2.normal * bary_v + v0.normal * (1 - bary_u - bary_v);

        const Vector uv = v1.uv * bary_u + v2.uv * bary_v + v0.uv * (1.0f - bary_u - bary_v);

        return Intersection {
            .distance = intersection_distance,
            .point = intersection_point,
            .flat_normal = flat_normal, .smooth_normal = smooth_normal,
            .uv = uv,
            .bary_u = bary_u, .bary_v = bary_v,
            .material_index = triangle.material_index
        };
    }

    return std::nullopt;
}

std::optional<Intersection> ray_intersect_triangle_span(const Ray &ray, std::span<const Triangle> triangles) {
    std::optional<Intersection> closest_intersection = std::nullopt;

    for (const auto &triangle : triangles) {
        if (auto intersection = ray_intersect_triangle(ray, triangle)) {
            if (!closest_intersection || intersection->distance < closest_intersection->distance) {
                closest_intersection = intersection;
            }
        }
    }
    
    return closest_intersection;
}

std::optional<Intersection> ray_intersect_acceleration_tree(const Ray &ray, const AccelerationTree &meshes) {
    return std::nullopt;
}

}