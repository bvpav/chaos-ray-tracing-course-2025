#include "crt_acceleration_tree.h"

#include <cassert>
#include <tuple>

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

static void build_branch(AccelerationTree &acceleration_tree, int parent_index, std::vector<Triangle> triangles, int depth) {
    if (depth > MAX_ACCELERATION_TREE_DEPTH || triangles.size() <= MAX_BOX_TRIANGLE_COUNT) {
        assert(acceleration_tree[parent_index].triangles.size() == 0);
        acceleration_tree[parent_index].triangles = std::move(triangles);
        return;
    }

    const auto [child0_bounds, child1_bounds] = acceleration_tree[parent_index].bounds.split(depth % 3); // Alternating the split axis

    std::vector<Triangle> &child0_triangles = triangles, child1_triangles;
    child1_triangles.reserve(triangles.size() / 2);

    auto child0_new_end = child0_triangles.begin();
    for (const auto &triangle : child0_triangles) {
        AABB triangle_bounds = get_triangle_aabb(triangle);
        bool is_in_child0 = child0_bounds.intersects(triangle_bounds);
        bool is_in_child1 = child1_bounds.intersects(triangle_bounds);

        if (!is_in_child0 && is_in_child1)
            child1_triangles.push_back(std::move(triangle));
        else if (is_in_child0 && is_in_child1)
        {
            *child0_new_end++ = triangle;
            child1_triangles.push_back(triangle);
        }
        else if (is_in_child0 && !is_in_child1)
            *child0_new_end++ = std::move(triangle);
    }

    child0_triangles.erase(child0_new_end, child0_triangles.end());

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
        build_branch(acceleration_tree, child0_index, std::move(child0_triangles), depth + 1);
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
        build_branch(acceleration_tree, child1_index, std::move(child1_triangles), depth + 1);
    }
}

AccelerationTree build(std::vector<Triangle> triangles) {
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
    build_branch(acceleration_tree, 0, std::move(triangles), 0);
    return acceleration_tree;
}

} // acceleration_tree

} // crt