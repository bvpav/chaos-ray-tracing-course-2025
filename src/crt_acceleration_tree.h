#pragma once

#include <span>
#include <vector>

#include "crt_aabb.h"
#include "crt_triangle.h"

namespace crt {

// Q: Are these good values?
inline constexpr int MAX_ACCELERATION_TREE_DEPTH = 10;
inline constexpr int MAX_BOX_TRIANGLE_COUNT = 2;

struct AccelerationTreeNode {
    std::vector<Triangle> triangles;
    AABB bounds;
    std::array<int, 2> children_indices;
    int parent_index;
};

using AccelerationTree = std::vector<AccelerationTreeNode>;

namespace acceleration_tree {

AccelerationTree build(std::span<const Triangle> triangles);

} // acceleration_tree

} // crt