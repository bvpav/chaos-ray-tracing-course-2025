#pragma once

#include <span>
#include <vector>

#include "crt_aabb.h"
#include "crt_triangle.h"

namespace crt {

struct AccelerationTreeNode {
    std::vector<Triangle> triangles;
    AABB bounds;
    int children_indices[2];
    int parent_index;
};

using AccelerationTree = std::vector<AccelerationTreeNode>;

namespace acceleration_tree {

AccelerationTree build(std::span<const Triangle> triangles);

} // acceleration_tree

} // crt