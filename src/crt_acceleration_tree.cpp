#include "crt_acceleration_tree.h"

#include <cassert>
#include <tuple>
#include <utility>
#include <vector>

#include "crt_aabb.h"
#include "crt_triangle.h"

namespace crt {

namespace acceleration_tree {

static constexpr void union_triangle_aabb(AABB &box, const Triangle &triangle) noexcept {
    std::apply([&](const auto &...v) {
        ((box.min.x = std::min(box.min.x, v.position.x),
            box.min.y = std::min(box.min.y, v.position.y),
            box.min.z = std::min(box.min.z, v.position.z),

            box.max.x = std::max(box.max.x, v.position.x),
            box.max.y = std::max(box.max.y, v.position.y),
            box.max.z = std::max(box.max.z, v.position.z)), ...);
    }, triangle.vertices());
}

static constexpr AABB get_triangle_aabb(const Triangle &triangle) noexcept {
    AABB result = AABB::vacuum();
    union_triangle_aabb(result, triangle);
    return result;
}

static void build_branch(AccelerationTree &acceleration_tree, int parent_index, std::span<const Triangle> triangle_span, int depth) {
    if (depth > MAX_ACCELERATION_TREE_DEPTH || triangle_span.size() <= MAX_BOX_TRIANGLE_COUNT) {
        assert(acceleration_tree[parent_index].triangles.size() == 0);
        acceleration_tree[parent_index].triangles = std::vector(triangle_span.begin(), triangle_span.end());
        return;
    }

    const auto [child0_bounds, child1_bounds] = acceleration_tree[parent_index].bounds.split(depth % 3); // Alternating the split axis

    std::vector<Triangle> child0_triangles, child1_triangles;

    for (const auto &triangle : triangle_span) {
        AABB triangle_bounds = get_triangle_aabb(triangle);
        if (child0_bounds.intersects(triangle_bounds)) 
            child0_triangles.push_back(triangle);
        if (child1_bounds.intersects(triangle_bounds)) 
            child1_triangles.push_back(triangle);
    }

    // TODO: compress these ifs
    if (child0_triangles.size() > 0) {
        int child0_index = acceleration_tree.size();
        acceleration_tree.emplace_back(AccelerationTreeNode {
            .triangles = {},
            .bounds = child0_bounds,
            .children_indices = { -1, -1 },
            .parent_index = parent_index
        });
        acceleration_tree[parent_index].children_indices[0] = child0_index;
        build_branch(acceleration_tree, child0_index, child0_triangles, depth + 1);
    }
    if (child1_triangles.size() > 0) {
        int child1_index = acceleration_tree.size();
        acceleration_tree.emplace_back(AccelerationTreeNode {
            .triangles = {},
            .bounds = child1_bounds,
            .children_indices = { -1, -1 },
            .parent_index = parent_index
        });
        acceleration_tree[parent_index].children_indices[1] = child1_index;
        build_branch(acceleration_tree, child1_index, child1_triangles, depth + 1);
    }
}

AccelerationTree build(std::span<const Triangle> triangles) {
    // Build bounding box, encapsulating the triangles
    AABB bounds = AABB::vacuum();

    // FIXME: we might be duplicating some vertices in the check
    for (const auto &triangle : triangles) {
        union_triangle_aabb(bounds, triangle);
    }

    AccelerationTree acceleration_tree;
    // Insert root node
    acceleration_tree.emplace_back(AccelerationTreeNode {
        .triangles = {},
        .bounds = bounds,
        .children_indices = { -1, -1 },
        .parent_index = -1,
    });
    build_branch(acceleration_tree, 0, triangles, 0);
    return acceleration_tree;
}

} // acceleration_tree

} // crt