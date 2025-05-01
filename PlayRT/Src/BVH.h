#pragma once

#include <vector>
#include <optional>
#include <variant>
#include <algorithm>
#include <iostream> 


#include "Math.h"
#include "Ray.h"


using namespace std;
 

//similar to small object optimization
constexpr uint32_t MAX_LEAF_SIZE = 4;

constexpr float EPSILON = 1e-6f;

struct alignas(16) Bounds3 {
    alignas(16) vec3 min{}, max{};

    Bounds3() {
		min = vec3(MAX_SCALAR_V);
		max = vec3(MIN_SCALAR_V); 
    }
};

inline bool AABB_intersect(const Bounds3& bounds, const Ray& ray) 
{
    vec3 invDir = ray.invertDir; 

    // 1. get the intersection of ray and the bounding box; vectorized; 
    vec3 t_max = (bounds.max - ray.origin) * invDir;
    vec3 t_min = (bounds.min - ray.origin) * invDir;

    for (int i = 0; i <= 2; i++) {
        if (ray.direction[i] < 0) {
            std::swap(t_min[i], t_max[i]); 
        }
    }
     
    //the ray intersects with the bounding box if the max t_min <  min t_max; 
    float min_exit = min3(t_max.x, t_max.y, t_max.z);
	float max_enter = max3(t_min.x, t_min.y, t_min.z);

	//cout << "max_enter: " << max_enter << " min_exit: " << min_exit << '\n';

    /*
    *  1. consider if the slab degenerate as a plane;
     allow enter = exit,  and tolerate numerical error; 
    2. only when ray exit from inside, min_exit <= 0; 
    */   
    if (min_exit > 0 && max_enter <= min_exit + EPSILON)
        return true;
    else
        return false; 

}

/*
* what geometry should provide in a RT pipeline
*/
class IRTPrimitive {
public: 
	virtual Bounds3 getBoundingBox() const = 0;
	 virtual optional<Intersection> intersect(const Ray& ray) const = 0;
};

template <typename T>
concept RTPrimitive = requires(T t, const Ray& ray) {
	{ t.getBoundingBox() } -> std::convertible_to<Bounds3>; 
    { t.intersect(ray) } -> std::convertible_to<std::optional<Intersection>>;
};



struct LeafNode {
    uint32_t first_prim; // Index of the first primitive in this leaf
    uint16_t prim_count; // Number of primitives in this leaf
};

// Define data specific to an internal node
struct InternalNode {
    uint32_t left_child; // Index of the left child node
};


struct alignas(32) BVHNode {
	Bounds3 bounds;           // Bounding box of the node
     
	variant<LeafNode, InternalNode> node_dataType;

    bool is_leaf() const {
        return  holds_alternative<LeafNode>(node_dataType);
    }
    const LeafNode& as_leaf() const {
        return std::get<LeafNode>(node_dataType);
    }

    const InternalNode& as_internal() const {
        return std::get<InternalNode>(node_dataType);
    }
}; 


template<RTPrimitive Primitive>
class BVH {
public:
    BVH(vector<Primitive>& prims):
		primitives(prims)
    {
        build(prims);
    }
     
	void build(vector<Primitive>& prims); 

	optional<Intersection> intersect(const Ray& ray) const;
    optional<Intersection> leaf_intersect(const BVHNode& node, const Ray& ray) const;

	vector<BVHNode> nodes;
     
	const vector<Primitive>& primitives;
};
 


inline uint8_t selectSplitAxisIndex(const Bounds3& bound) {
    auto min = bound.min;
    auto max = bound.max;
    vec3 extents{ max[0] - min[0], max[1] - min[1], max[2] - min[2] };

    if (extents.x >= extents.y && extents.x >= extents.z) {
        return 0;
    }
    else if (extents.y >= extents.z) {
        return 1;
    }
    else {
        return 2;
    }
}

//similar to quicksort partitioning
template<RTPrimitive Primitive>
inline uint32_t partition_primitives(std::vector<Primitive>& prims, uint32_t start, uint32_t end, uint8_t axis) {
    
	//the partitioning value is the average of the centroids of the primitives
    double split_value = 0.0;
    for (uint32_t i = start; i < end; ++i) {
        auto [prim_min, prim_max] = prims[i].getBoundingBox();
        split_value += 0.5 * (prim_min[axis] + prim_max[axis]);
    }
    split_value /= (end - start); 

    auto mid_iter = std::partition(prims.begin() + start, prims.begin() + end,
        [axis, split_value](const Primitive& prim) {
            auto [prim_min, prim_max] = prim.getBoundingBox();
            double centroid = 0.5 * (prim_min[axis] + prim_max[axis]);
            return centroid < split_value;
        });

    uint32_t mid = static_cast<uint32_t>(std::distance(prims.begin(), mid_iter));

    // Handle cases where partitioning fails
    if (mid == start || mid == end) {
        mid = start + (end - start) / 2;
    }

    return mid;
}



template<RTPrimitive Primitive>
inline Bounds3 UnionBounds(const std::vector<Primitive>& primitives, uint32_t start, uint32_t end) {
    Bounds3 retBounds;
    for (uint32_t i = start; i < end; ++i) {
        auto thisBound = primitives[i].getBoundingBox();
        retBounds.min = min(retBounds.min, thisBound.min);
        retBounds.max = max(retBounds.max, thisBound.max);
    }
    return retBounds;
}

 

template<RTPrimitive Primitive>
void BVH<Primitive>::build(vector<Primitive>& primitives) {
    if (primitives.empty()) return;

    nodes.clear();
    nodes.reserve(primitives.size() * 2); // Reserve space to minimize reallocations

    struct BuildTask {
        uint32_t node_index;
        uint32_t start_primIndex;
        uint32_t end_primIndex;
    };

    // Initialize root node
    nodes.emplace_back();
    BuildTask root_task = { 0, 0, static_cast<uint32_t>(primitives.size()) };
    vector<BuildTask> taskStack = { root_task };

    while (!taskStack.empty()) {
        BuildTask task = taskStack.back();
        taskStack.pop_back();

        BVHNode& node = nodes[task.node_index];

        //recompute bounds
        node.bounds = UnionBounds<Primitive>(primitives, task.start_primIndex, task.end_primIndex);

        uint32_t prim_count = task.end_primIndex - task.start_primIndex;

        // Leaf node
        if (prim_count <= MAX_LEAF_SIZE) { 
			node.node_dataType = LeafNode{ task.start_primIndex, static_cast<uint16_t>(prim_count) };
			cout << "built leaf node" << '\n';
            continue;
        }
         
            // Internal node 
        auto axis = selectSplitAxisIndex(node.bounds); 

        uint32_t mid = partition_primitives<Primitive>(primitives, task.start_primIndex, task.end_primIndex, axis);

        // Create child nodes
        uint32_t left_child_index = static_cast<uint32_t>(nodes.size());
        nodes.emplace_back();
        uint32_t right_child_index = static_cast<uint32_t>(nodes.size());
        nodes.emplace_back();

		node.node_dataType = InternalNode{ left_child_index };

        // Push tasks for child nodes
        taskStack.push_back({ left_child_index, task.start_primIndex, mid });
        taskStack.push_back({ right_child_index, mid, task.end_primIndex });
        
    }

}

template<RTPrimitive Primitive>
std::optional<Intersection> BVH<Primitive>::leaf_intersect(const BVHNode& node, const Ray& ray) const {
    auto _node = node.as_leaf();
    auto closest_t = ray.t_max;
    std::optional<Intersection> closest_intersection;
 
    for (uint32_t i = _node.first_prim; i < _node.first_prim + _node.prim_count; ++i) {
        const auto& primitive = primitives[i];

        // Check for intersection with the primitive
        if (auto intersection = primitive.intersect(ray); intersection.has_value() 
			&& intersection.value().travel_t < closest_t)
        {
            // Update closest intersection
			closest_t = intersection.value().travel_t;
			closest_intersection = intersection;
        }
    }

    return closest_intersection; // Returns the closest intersection or std::nullopt if none found
}

template<RTPrimitive Primitive>
inline optional<Intersection> BVH<Primitive>::intersect(const Ray& ray) const
{
    std::optional<Intersection> closest;
    float closest_t = ray.t_max;

    // 使用显式栈来进行非递归遍历
    std::vector<uint32_t> stack;
    stack.push_back(0); // 从根节点开始

    while (!stack.empty()) {
        uint32_t node_index = stack.back();
        stack.pop_back();
        const BVHNode& node = nodes[node_index];

        // 检查光线是否与该节点的包围盒相交
        if (!AABB_intersect(node.bounds, ray)) {
            //cout << "miss AABB" << '\n';
            continue;
        } 

        if (node.is_leaf()) {
			// 叶子节点，测试其包含的所有物体 
			//cout << "hit leaf" << '\n';
            if (auto intersection = leaf_intersect(node, ray); intersection.has_value() 
                && intersection.value().travel_t < closest_t)
            { 
                // Update the closest intersection
                closest_t = intersection.value().travel_t;
                closest = intersection;
            }  
        }
        else {
            //cout << "hit internal" << '\n';
            // 内部节点，将子节点压入栈中
			auto left_child = node.as_internal().left_child;
            stack.push_back(left_child);
            stack.push_back(left_child + 1);
        }
    }

    return closest; 

}

 